#include "StarAuthenticationConnection.hpp"
#include "StarClock.hpp"
#include "StarTcp.hpp"

namespace Star {
namespace Auth {

AuthenticationConnection::AuthenticationConnection(String const& authServer, int port) {
  m_authServer = authServer;
  m_port = port;
}

String AuthenticationConnection::query(String const& request) {
  auto socket = TcpSocket::connectTo(HostAddress(m_authServer, m_port));
  socket->setNoDelay(true);
  socket->setSocketTimeout(AuthRequestTimeout);

  String requestHeader = "POST /auth.sb HTTP/1.1\r\nHost: " + m_authServer + "\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n";
  socket->writeFull(requestHeader.utf8Ptr(), requestHeader.utf8Size());
  socket->writeFull(request.utf8Ptr(), request.utf8Size());
  String requestFooter = "\r\n\r\n";
  socket->writeFull(requestFooter.utf8Ptr(), requestFooter.utf8Size());
  socket->flush();

  int64_t deadline = Clock::currentTime() + AuthRequestTimeout;
  Buffer buffer(ResponseBufferCapacity);
  bool headerMode = true;
  int newlineCounter = 0;
  while (socket->isOpen()) {
    if (Clock::currentTime() >= deadline) {
      throw StarException("Timeout reading auth server response.");
    }
    char c;
    socket->readFull(&c, 1);
      throw StarException("Unexpected result from socket read.");
    if (c == '\r')
      continue;
    buffer.writeFull(&c, 1);
    if (buffer.pos() >= ResponseBufferCapacity) {
      throw StarException("Auth server response too long.");
    }
    if (c == '\n')
      newlineCounter++;
    else
      newlineCounter = 0;
    if (newlineCounter == 2) {
      if (headerMode) {
        String header = String(buffer.ptr(), buffer.pos());
        if (!header.beginsWith("HTTP/1.1 200 OK\n")) {
          throw StarException("Auth server invalid response.");
        }
        headerMode = false;
        buffer.clear();
        newlineCounter = 0;
      }
      else {
        return String(buffer.ptr(), buffer.pos() - 2); // remove trailing newlines, guaranteed to be there
      }
    }
  }
  throw StarException("Connection lost.");
}

}
}
