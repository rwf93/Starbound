#include "StarUniverseController.hpp"
#include "StarRoot.hpp"
#include "StarException.hpp"
#include "StarBuffer.hpp"
#include "StarJsonVariant.hpp"
#include "StarPerformance.hpp"
#include "StarLogging.hpp"

namespace Star {

UniverseController::UniverseController(TcpSocketPtr socket) : Thread("UniverseController") {
  m_socket = socket;
  m_parent = 0;
  m_stop = false;
  m_closed = false;
}

void UniverseController::stop() {
  MutexLocker lock(m_lock);
  m_stop = true;
  m_condition.broadcast();
  m_socket->close();
}

bool UniverseController::isClosed() {
  return m_closed;
}

void UniverseController::bind(UniverseControllerSet* parent) {
  m_parent = parent;
}

void UniverseController::receiveResponse(Variant const& response) {
  MutexLocker lock(m_lock);
  m_responses.append(response);
  m_condition.broadcast();
}

void UniverseController::run() {
  try {
    Buffer buffer(1024);
    MutexLocker lock(m_lock);
    while (!m_stop && m_socket->isOpen()) {
      buffer.clear();
      List<int> modeStack;
      bool trivial = true;
      int mode = 0;

      /*
       * Mode list
       * 0 primitive mode, accepts json or a single line of text which is interpreted as list of strings
       * 1 list mode
       * 20 object initial mode
       * 21 object next mode
       * 22 object colon mode
       * 3 string mode  31 espaced char in string mode
       * 4 value mode
       * 5 literal mode
       * 6 done, but search for newline
       * -1 complete request
       * -2 syntax error
       *
       */

      while (mode >= 0) {
        char c;
        if (m_socket->read(&c, 1) != 1)
          throw StarException("Unexpected result from socket read\n");
        if (c == '\r')
          continue; // hello dos
        buffer.write(&c, 1);
        revisit: if ((mode == 0) || (mode == 1) || (mode == 20) || (mode == 4) || (mode == 6)) {
          if ((c == ' ') || (c == '\t')) {
            // trim whitespace
            continue;
          }
        }
        if ((mode == 1) || (mode == 20) || (mode == 4)) {
          if (c == '\n') {
            // trim newlines
            continue;
          }
        }
        switch (mode) {
        case 0: { // primitive mode
          if (c == '\n') {
            mode = -1; //empty line, meh
          } else if (c == '[') {
            modeStack.push_back(6);
            modeStack.push_back(1);
            trivial = false;
            mode = 4;
          } else if (c == '{') {
            modeStack.push_back(6);
            modeStack.push_back(20);
            trivial = false;
            mode = 4;
          } else if (c == '"') {
            modeStack.push_back(6);
            mode = 3;
          } else {
          }
          break;
        }
        case 1: { // list mode
          if (c == ',') {
            modeStack.push_back(1);
            mode = 4;
          } else if (c == ']') {
            mode = modeStack.takeLast();
          } else
            mode = -2;
          break;
        }
        case 20: { // object
          if (c == '}') {
            mode = modeStack.takeLast();
          } else {
            modeStack.push_back(22);
            mode = 4;
            goto revisit;
          }
          break;
        }
        case 21: { // continue bit of object pairs
          if (c == '}') {
            mode = modeStack.takeLast();
          } else if (c == ',') {
            modeStack.push_back(22);
            mode = 4;
          }
          break;
        }
        case 22: { // colon bit of object pairs
          if (c == ':') {
            modeStack.push_back(21);
            mode = 4;
          } else {
            mode = -2;
          }
          break;
        }
        case 3: { // string mode
          if (c == '\\')
            mode = 31;
          else if (c == '"')
            mode = modeStack.takeLast();
          break;
        }
        case 31: { // escaped in string mode
          mode = 3;
          break;
        }
        case 4: { // value mode
          if (c == '[') {
            modeStack.push_back(mode);
            modeStack.push_back(1);
            trivial = false;
            mode = 4;
          } else if (c == '{') {
            modeStack.push_back(mode);
            modeStack.push_back(20);
            trivial = false;
            mode = 4;
          } else if (c == '"') {
            modeStack.push_back(mode);
            mode = 3;
          } else {
            mode = 5;
            goto revisit;
          }
          break;
        }
        case 5: { // literal mode
          if ((c == ' ') || (c == '\t') || (c == '\n') || (c == ',') || (c == ']') || (c == '}') || (c == ':')) {
            mode = modeStack.takeLast();
            goto revisit;
          }
          break;
        }
        case 6: {
          if (c == '\n') {
            mode = -1;
          } else {
            mode = -2;
          }
          break;
        }
        }
      }

      bool hasCommand = false;
      VariantList request;

      if (mode == -2) {
        while (true) {
          char c;
          if (m_socket->read(&c, 1) != 1)
            throw StarException("Unexpected result from socket read\n");
          if (c != '\n')
            continue;
        }

        VariantList failure;
        failure.append("syntaxError");
        m_responses.push_back(failure);
      } else {
        String requestStr = String(buffer.ptr(), buffer.dataSize());
        if (requestStr.size() == 0) {
          VariantList failure;
          failure.append("typeHelpForMoreInfo");
          m_responses.push_back(failure);
        } else if (trivial) {
          VariantList parts;
          for (auto part : requestStr.splitWhitespace()) {
            if (part.size() > 0)
              parts.append(part);
          }
          if (parts.size() == 0) {
            VariantList failure;
            failure.append("typeHelpForMoreInfo");
            m_responses.push_back(failure);
          } else {
            request = parts;
            hasCommand = true;
          }
        } else {
          try {
            Variant parseResult = Variant::parse(requestStr);
            bool valid = true;
            if (valid && (parseResult.type() != Variant::Type::LIST))
              valid = false;
            if (valid)
              request = parseResult.toList();
            if (valid && (request.size() < 1))
              valid = false;
            if (valid && (request[0].type() != Variant::Type::STRING))
              valid = false;

            if (!valid) {
              VariantList failure;
              failure.append("parseError");
              failure.append("Must be a list with the first element of string type");
              m_responses.push_back(failure);
            } else {
              hasCommand = true;
            }
          } catch (const std::exception& e) {
            VariantList failure;
            failure.append("parseError");
            failure.append(e.what());
            m_responses.push_back(failure);
          }
        }
      }

      if (hasCommand) {
        m_parent->commandReceived(make_shared<ControllerCommand>(shared_from_this(), request));
      }
      if (hasCommand)
        while ((m_responses.size() == 0) && (!m_stop))
          m_condition.wait(m_lock);
      for (auto response : m_responses) {
        buffer.clear();
        outputUtf8Json(response, IODeviceOutputIterator(&buffer), 0, false);
        char c = '\n';
        buffer.write(&c, 1);
        if (m_socket->write(buffer.ptr(), buffer.dataSize()) != buffer.dataSize())
          throw StarException("Unexpected result from socket write\n");
        m_socket->flush();
      }
      m_responses.clear();
    }
  } catch (const NetworkException& e) {
    goto finally;
  } catch (const std::exception& e) {
    Logger::error("UniverseController: Exception caught: %s", e.what());
    goto finally;
  } catch (...) {
    goto finally;
  }
  finally: {
    m_closed = true;
  }
  m_parent->notify();
}

UniverseControllerSet::UniverseControllerSet(UniverseServer* universeServer): Thread("UniverseControllerSet") {
  m_universeServer = universeServer;
  m_stop = false;
  m_active = true;
}

void UniverseControllerSet::addNewConnection(UniverseControllerPtr controller) {
  MutexLocker lock(m_lock);
  if (m_stop)
    throw StarException("Shutting down");
  controller->bind(this);
  m_controllers.add(controller);
  m_active = true;
  m_condition.broadcast();
  controller->start();
}

void UniverseControllerSet::stop() {
  MutexLocker lock(m_lock);
  m_stop = true;
  m_active = true;
  m_condition.broadcast();
}

void UniverseControllerSet::notify() {
  MutexLocker lock(m_lock);
  m_condition.broadcast();
}

void UniverseControllerSet::commandTypeHelpForMoreInfo(ControllerCommandPtr command) {
  VariantList result;
  result.append("todo");
  command->respond(result);
}

void UniverseControllerSet::commandCounters(ControllerCommandPtr command) {
  VariantList result;
  result.append("counters");

  VariantMap config;
  if (command->request().size() >=2)
    config = command->request()[1].toMap();

  bool reset = config.value("reset", true).toBool();

  auto assets = Root::singleton().assets();
  for (auto records : assets->variant("/perf.config:parenting").list())
    Performance::setCounterHierarchy(records.getString(1).utf8Ptr(), records.getString(0).utf8Ptr());

  result.append(Star::Performance::dumpToVariant(reset));

  command->respond(result);
}

void UniverseControllerSet::commandUnknown(ControllerCommandPtr command) {
  VariantList result;
  result.append("unkownCommand");
  result.append(command->command());
  command->respond(result);
}

void UniverseControllerSet::run() {
  while (!m_stop) {
    {
      MutexLocker lock(m_lock);
      m_active = false;
    }
    Set<UniverseControllerPtr> closed;
    {
      MutexLocker lock(m_lock);
      for (auto iter = m_controllers.begin(); iter != m_controllers.end(); iter++) {
        auto controller = *iter;
        if (controller->isClosed())
          closed.add(controller);
      }
    }
    for (auto controller : closed) {
      m_controllers.remove(controller);
      controller->stop();
      controller->join();
    }
    List<ControllerCommandPtr> commands;
    {
      MutexLocker lock(m_lock);
      for (auto command : m_commandQueue)
        commands.push_back(command);
      m_commandQueue.clear();
    }
    for (auto command : commands) {
      if (command->command() == "typeHelpForMoreInfo")
        commandTypeHelpForMoreInfo(command);
      else if (command->command() == "counters")
        commandCounters(command);
      else if (command->command() == "worlds")
        m_universeServer->commandReceived(command);
      else if (command->command() == "world")
        m_universeServer->commandReceived(command);
      else
        commandUnknown(command);
    }
    {
      MutexLocker lock(m_lock);
      if (!m_active)
        m_condition.wait(m_lock);
    }
  }
  for (auto iter = m_controllers.begin(); iter != m_controllers.end(); iter++) {
    auto controller = *iter;
    controller->stop();
    controller->join();
  }
  m_controllers.clear();
}

void UniverseControllerSet::commandReceived(ControllerCommandPtr command) {
  MutexLocker lock(m_lock);
  m_commandQueue.push_back(command);
  m_active = true;
  m_condition.broadcast();
}

ControllerCommand::ControllerCommand(weak_ptr<UniverseController> controller, VariantList request) {
  m_controller = controller;
  m_request = request;
  m_command = request[0].toString();
}

void ControllerCommand::popOuterCommand(int innerIndex)
{
  m_request = m_request[innerIndex].toList();
  m_command = m_request[0].toString();
}

String const& ControllerCommand::command() {
  return m_command;
}

VariantList& ControllerCommand::request() {
  return m_request;
}

void ControllerCommand::respond(VariantList const& response) {
  if (auto controller = m_controller.lock())
    controller->receiveResponse(response);
}

}
