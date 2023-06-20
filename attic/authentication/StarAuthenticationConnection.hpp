#ifndef _STAR_AUTHENTICATION_CONNECTION_HPP_
#define _STAR_AUTHENTICATION_CONNECTION_HPP_

#include "StarVariant.hpp"

namespace Star {

namespace Auth {

static int const AuthRequestTimeout = 10*1000;
static int const ResponseBufferCapacity = 15*1024;

class AuthenticationConnection {
public:
  AuthenticationConnection(String const& authServer, int port);

  String query(String const& request);

private:
  String m_authServer;
  int m_port;
};

}

}

#endif
