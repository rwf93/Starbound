#include "StarServerAuthentication.hpp"
#include "StarAuthenticationKey.hpp"
#include <ctime>


namespace Star {
namespace Auth {

ServerAuthentication::ServerAuthentication(String const& rootPublicKey) {
  m_rootPublicKey.loadPublicKey(rootPublicKey);
}

ServerAuthentication::~ServerAuthentication() {}

String ServerAuthentication::authsvrRequest(String const& username, String const& password, String const& hostname, int port, String const& authCertificate) {
  //le todo
  _unused(username);
  _unused(password);
  _unused(hostname);
  _unused(port);
  _unused(authCertificate);
  return String();
}

bool ServerAuthentication::authsvrResponse(String const& response) {
  _unused(response);
  return false;
}

String ServerAuthentication::clientRequest(String const& request) {
  _unused(request);
  return String();
}

}

}
