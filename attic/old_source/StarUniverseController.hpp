#ifndef STAR_UNIVERSE_CONTROLLER_HPP
#define STAR_UNIVERSE_CONTROLLER_HPP

#include <memory>
#include "StarConfig.hpp"
#include "StarUniverseServer.hpp"
#include "StarTcp.hpp"
#include "StarThread.hpp"

namespace Star {

using namespace std;

class UniverseControllerSet;

class UniverseController : public Thread, public enable_shared_from_this<UniverseController> {
public:
  UniverseController(TcpSocketPtr socket);
  void stop();
  bool isClosed();

  void bind(UniverseControllerSet* parent);

  void receiveResponse(Variant const&response);

  virtual void run();
private:
  TcpSocketPtr m_socket;
  UniverseControllerSet* m_parent;
  bool m_stop;
  bool m_closed;

  Mutex m_lock;
  ConditionVariable m_condition;

  List<Variant> m_responses;

};
typedef shared_ptr<UniverseController> UniverseControllerPtr;

class ControllerCommand;
typedef shared_ptr<ControllerCommand> ControllerCommandPtr;

class UniverseControllerSet : public Thread {
public:
  UniverseControllerSet(UniverseServer* universeServer);

  void addNewConnection(UniverseControllerPtr controller);

  void commandReceived(ControllerCommandPtr command);

  virtual void run();
  void stop();
  void notify();

private:
  UniverseServer* m_universeServer;

  bool m_stop;
  Mutex m_lock;
  bool m_active;
  ConditionVariable m_condition;
  Set<UniverseControllerPtr> m_controllers;
  List<ControllerCommandPtr> m_commandQueue;

  void commandTypeHelpForMoreInfo(ControllerCommandPtr command);
  void commandCounters(ControllerCommandPtr command);
  void commandUnknown(ControllerCommandPtr command);

};
typedef shared_ptr<UniverseControllerSet> UniverseControllerSetPtr;

class ControllerCommand {
public:
  ControllerCommand(weak_ptr<UniverseController> controller, VariantList request);
  String const& command();
  VariantList& request();
  void respond(VariantList const& response);
  void popOuterCommand(int innerIndex);
private:
  weak_ptr<UniverseController> m_controller;
  String m_command;
  VariantList m_request;
};

}

#endif
