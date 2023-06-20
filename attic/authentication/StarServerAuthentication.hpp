#ifndef _STAR_SERVER_AUTHENTICATION_HPP_
#define _STAR_SERVER_AUTHENTICATION_HPP_

#include "StarAuthenticationKey.hpp"
#include "StarVariant.hpp"
#include <string>

namespace Star {

namespace Auth {

class ServerAuthentication {
public:
    ServerAuthentication(String const& rootPublicKey);
    ~ServerAuthentication();
    String authsvrRequest(String const& username, String const& password, String const& hostname, int port, String const& authCertificate);
    bool authsvrResponse(String const& response);
    String clientRequest(String const& request);

private:
    Auth::Key m_serverPrivateKey;
    Auth::Key m_authPublicKey;
    Auth::Key m_rootPublicKey;
    Variant m_authCertificate;
    Variant m_serverCertificate;
};

}

}

#endif
