#include "StarAuthenticationServer.hpp"
#include "StarLogging.hpp"
#include "StarAuthenticationDatabase.hpp"

namespace Star {
namespace Auth {

AuthenticationConnectionHandler::AuthenticationConnectionHandler(TcpSocketPtr socket, AuthenticationServer* server)
    : Thread("AuthenticationConnectionHandler") {
  Logger::info("Connection thread constructor.");
  m_socket = socket;
  m_server = server;
  m_stop = false;
  m_closed = false;
}

AuthenticationConnectionHandler::~AuthenticationConnectionHandler() {
  Logger::info("Connection thread destructor.");
}

void AuthenticationConnectionHandler::stop() {
  m_stop = true;
  try {
    m_socket->close();
  } catch (StarException const&) {
    // bartwe: I'm a jerk, close is not supposed to be called during calls to the socket on other threads, not really seeing a good way to interrupt that i care about enough to implement atm.
  }
}

bool AuthenticationConnectionHandler::isClosed() {
  return m_closed;
}

void AuthenticationConnectionHandler::writeResponse(String const& response) {
  String body = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, no-store\r\n\r\n" + response + "\r\n\r\n";
  m_socket->write(body.utf8Ptr(), body.utf8Size());
  Logger::info("Connection thread write response.");
}

void AuthenticationConnectionHandler::writeErrorResponse(String const& response) {
  String body = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, no-store\r\n\r\n" + response + "\r\n\r\n";
  m_socket->write(body.utf8Ptr(), body.utf8Size());
  Logger::info("Connection thread write error.");
}

void AuthenticationConnectionHandler::run() {
  Logger::info("Connection thread.run");
  try {
    m_socket->setNoDelay(true);
    m_socket->setSocketTimeout(ClientSocketTimeout);

    int64_t deadline = Thread::currentTime() + ClientSocketTimeout;

    bool headerMode = true;
    Buffer buffer(BufferCapacity);
    int newlineCounter = 0;
    while (!m_stop && m_socket->isOpen()) {
      if (Thread::currentTime() >= deadline) {
        // client too slow, or a slow lorris attack
        writeErrorResponse("Client too slow.");
        break;
      }
      char c;
      if (m_socket->read(&c, 1) != 1)
        throw StarException("Unexpected result from socket read\n");
      if (c == '\r')
        continue; // hello dos
      buffer.write(&c, 1);
      if (buffer.pos() >= BufferCapacity) {
        // request too long
        writeErrorResponse("Request too long.");
        break;
      }
      if (c == '\n')
        newlineCounter++;
      else
        newlineCounter = 0;
      if (newlineCounter == 2) {
        if (headerMode) {
          String header = String(buffer.ptr(), buffer.pos());
          if (!header.beginsWith("POST /auth.sb HTTP/1.1\n")) {
            writeErrorResponse("Page not found.");
            break;
          }
          headerMode = false;
          buffer.clear();
          newlineCounter = 0;
        } else {
          String body = String(buffer.ptr(), buffer.pos() - 2); // remove trailing newlines, guaranteed to be there
          auto message = make_shared<MessageToken>(body);
          Logger::info("Connection thread.message received.");
          m_server->addMessage(message);
          if (!message->waitForResponse(ClientProcessingTimeout))
            writeErrorResponse("Timed out.");
          writeResponse(message->response());
          break;
        }
      }
    }
    m_socket->flush();
  }
  catch (NetworkException const&) {
    goto finally;
  }
  catch (StarException const& e) {
    Logger::error("AuthenticationConnectionHandler: Exception caught: %s", e.what());
  }
  catch (std::exception const& e) {
    Logger::error("AuthenticationConnectionHandler: Exception caught: %s", e.what());
    goto finally;
  }
  catch (...) {
    goto finally;
  }
  finally: {
    m_closed = true;
    if (m_socket->isOpen())
      m_socket->close();
  }
  m_server->addMessage( { });
  Logger::info("Connection thread.run.end");
}

AuthenticationServer::AuthenticationServer() {
  m_stop = false;

  String connectionString = Variant::parseJson(File::readFileString("connectionstring.config")).getString("connectionString");

  DatabasePtr db = as<Database>(make_shared<AuthenticationDatabase>(connectionString));
  m_service = make_shared<AuthenticationService>(db);

  m_requestCount = 0;
  m_getAuthKeyCounter = 0;
  m_authorizeClientCounter = 0;
  m_authorizeClientFailure = 0;
  m_validateClientCounter = 0;
  m_validateClientFailure = 0;

  m_munin = make_shared<MuninNode>(Variant::parseJson(File::readFileString("authmunin.config")));
  m_munin->setCallback([=]() {
    MutexLocker locker(m_mainLock);
    m_munin->set("sbauth", "users", db->usersCount());
    m_munin->set("sbauth", "requests", m_requestCount);
    m_munin->set("sbauth", "getAuthKey", m_getAuthKeyCounter);
    m_munin->set("sbauth", "authorizeClient", m_authorizeClientCounter);
    m_munin->set("sbauth", "authorizeClientFailure", m_authorizeClientFailure);
    m_munin->set("sbauth", "validateClient", m_validateClientCounter);
    m_munin->set("sbauth", "validateClientFailure", m_validateClientFailure);
  });
}

AuthenticationServer::~AuthenticationServer() {}

void AuthenticationServer::acceptConnection(TcpSocketPtr socket) {
  MutexLocker locker(m_mainLock);
  if (m_stop) {
    socket->close();
  } else {
    auto handler = make_shared<AuthenticationConnectionHandler>(socket, this);
    m_handlers.add(handler);
    handler->start();
  }
}

void AuthenticationServer::run() {
  TcpServer tcpServer(ListenPort);
  tcpServer.setAcceptCallback(std::bind(&AuthenticationServer::acceptConnection, this, std::placeholders::_1));

  Logger::info("Auth server started.");

  List<AuthenticationConnectionHandlerPtr> cleanup;
  while (true) {
    try {
      MessageTokenPtr message;
      {
        MutexLocker locker(m_mainLock);
        if (m_stop)
          break;
        for (auto& handler : m_handlers) {
          if (handler->isClosed())
            cleanup.append(handler);
        }
        m_handlers.removeAll(cleanup);
        if (m_messages.empty() && cleanup.empty())
          m_signal.wait(m_mainLock);
        if (!m_messages.empty())
          message = m_messages.takeFirst();
      }
      cleanup.clear();
      if (message) {
        try {
          handleMessage(message);
          if (!message->hasResponse())
            message->setResponse("");
        }
        catch (std::exception const&) {
          if (!message->hasResponse())
            message->setResponse("");
          throw;
        }
      }
    }
    catch (std::exception const& e) {
      Logger::error("AuthenticationServer exception caught: %s", e.what());
    }
  }

  Logger::info("Finishing remaining connections.");

  try {
    MutexLocker locker(m_mainLock);
    m_stop = true;
    tcpServer.close();
  } catch (std::exception const& e) {
    Logger::error("AuthenticationServer exception caught cleaning up: %s", e.what());
  }

  {
    MutexLocker locker(m_mainLock);
    for (auto & message : m_messages)
      message->setResponse("stopping");
    m_messages.clear();
  }

  // locking should no longer be needed at this point
  // unused to avoid deadlock scenario where the handler calls addMessage
  for (auto& handler : m_handlers)
    handler->stop();
  m_handlers.clear();
}

void AuthenticationServer::stop() {
  Logger::info("Auth server stopping.");
  MutexLocker locker(m_mainLock);
  m_stop = true;
  m_signal.signal();
}

void AuthenticationServer::addMessage(MessageTokenPtr const& message) {
  MutexLocker locker(m_mainLock);
  m_requestCount++;
  if (m_stop) {
    if (message)
      message->setResponse("stopped");
  } else {
    m_messages.append(message);
    m_signal.signal();
  }
}

void AuthenticationServer::handleMessage(MessageTokenPtr const& message) {
  Variant command = Variant::parse(message->request());
  if (command.getString("command") == "getAuthKey") {
    m_getAuthKeyCounter++;
    auto response = m_service->getCertificate();
    message->setResponse(response.repr(0, true));
  } else if (command.getString("command") == "authorizeClient") {
    m_authorizeClientCounter++;
    try {
      auto response = m_service->authorizeClient(command.getMap("body"));
      message->setResponse(response.repr(0, true));
    }
    catch (...) {
      m_authorizeClientFailure++;
      throw;
    }
  } else if (command.getString("command") == "validateClient") {
    m_validateClientCounter++;
    try {
      auto response = m_service->validateClient(command.getMap("body"));
      message->setResponse(response.repr(0, true));
    }
    catch (...) {
      m_validateClientFailure++;
      throw;
    }
  }
}

MessageToken::MessageToken(String const& request) {
  m_request = request;
  m_hasResponse = false;
}

bool MessageToken::hasResponse() {
  MutexLocker locker(m_lock);
  return m_hasResponse;
}

bool MessageToken::waitForResponse(unsigned millis) {
  MutexLocker locker(m_lock);
  int64_t timeout = millis;
  while (true) {
    if (m_hasResponse)
      return true;
    if (timeout <= 0)
      return false;
    int64_t startTime = Thread::currentTime();
    m_signal.wait(m_lock, (unsigned)timeout);
    timeout -= Thread::currentTime() - startTime;
  }
}

void MessageToken::setResponse(String const& response) {
  {
    MutexLocker locker(m_lock);
    starAssert(!m_hasResponse);
    m_response = response;
    m_hasResponse = true;
  }
  m_signal.signal();
}

String MessageToken::response() {
  MutexLocker locker(m_lock);
  starAssert(m_hasResponse);
  return m_response;
}

String MessageToken::request() {
  return m_request;
}

}
}
