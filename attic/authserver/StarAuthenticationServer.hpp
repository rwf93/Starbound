#ifndef _STAR_AUTHENTICATOR_SERVER_HPP_
#define _STAR_AUTHENTICATOR_SERVER_HPP_

#include "StarAuthenticationKey.hpp"
#include "StarAuthenticationService.hpp"
#include "StarException.hpp"
#include "StarVariant.hpp"
#include "StarRoot.hpp"
#include "StarTcp.hpp"
#include "StarThread.hpp"
#include "StarMunin.hpp"

namespace Star {

namespace Auth {

//todo, port 80, hide behind a normal httpd, etc.
 static int const ListenPort = 21027;
static unsigned const ClientSocketTimeout = 10000; // 10 seconds
static unsigned const ClientProcessingTimeout = 30000; // 30 seconds
static int const BufferCapacity = 15*1024;

class AuthenticationServer;
typedef shared_ptr<AuthenticationServer> AuthenticationServerPtr;

//todo: bad on purpose, replace with a httpd frontend and deliver zmq messages
// interops with the main thread using MessageToken, suitable to attach to zmq style, think mongrel httpd
class AuthenticationConnectionHandler: public Thread {
public:
  AuthenticationConnectionHandler(TcpSocketPtr socket, AuthenticationServer* server);
  ~AuthenticationConnectionHandler();

  // requests a stop, doesn't actually stop the, for that you'll need to finalize or join()
  // which may take a long while due to socket operations not being interrupted.
  void stop();
  bool isClosed();

  virtual void run();

private:
  TcpSocketPtr m_socket;
  AuthenticationServer* m_server;
  bool m_stop;
  bool m_closed;

  Mutex m_lock;

  void writeResponse(String const& response);
  void writeErrorResponse(String const& response);
};
typedef shared_ptr<AuthenticationConnectionHandler> AuthenticationConnectionHandlerPtr;

class MessageToken {
public:
  MessageToken(String const& request);

  bool hasResponse();
  bool waitForResponse(unsigned millis);

  void setResponse(String const& response);

  String request();
  String response();

private:
  Mutex m_lock;
  ConditionVariable m_signal;
  bool m_hasResponse;
  String m_request;
  String m_response;
};
typedef shared_ptr<MessageToken> MessageTokenPtr;

class AuthenticationServer {
public:
  AuthenticationServer();
  ~AuthenticationServer();

  void run();
  void stop();

  void addMessage(MessageTokenPtr const& message);

private:
  shared_ptr<AuthenticationService> m_service;

  Set<AuthenticationConnectionHandlerPtr> m_handlers;
  List<MessageTokenPtr> m_messages;

  ConditionVariable m_signal;
  Mutex m_mainLock;
  bool m_stop;
  MuninNodePtr m_munin;
  int m_requestCount;
  int m_getAuthKeyCounter;
  int m_authorizeClientCounter;
  int m_authorizeClientFailure;
  int m_validateClientCounter;
  int m_validateClientFailure;

  void acceptConnection(TcpSocketPtr socket);

  void handleMessage(MessageTokenPtr const& message);
};


}

}

#endif
