#ifndef _STAR_CLIENT_AUTHENTICATION_HPP_
#define _STAR_CLIENT_AUTHENTICATION_HPP_

#include "StarAuthenticationKey.hpp"
#include "StarException.hpp"
#include "StarVariant.hpp"
#include <string>

namespace Star {

namespace Auth {

/*

<AuthSignature>=
 {
   "signedAuthKey" : {
     "authPublicKey": ...
     "validFrom":...
     "validTo":...
   }
   "rootSignedAuthKeySignature": <signature made with root privkey>
}

<ClientClaim>=
{
  "claim": {
    "metadata: {
      "clientPublicKey":...
      "username":...
      "validFrom":...
      "validTo":...
    }
    "signature"
  }
  "authSignature": {
    "signedAuthKey" : {
      "authPublicKey": ...
      "validFrom":...
      "validTo":...
    }
    "rootSignedAuthKeySignature": <signature made with root privkey>
  }
  "signature": <signature of main message with authPrivateKey>
}


 */

class ClientAuthentication {
public:
  ClientAuthentication(shared_ptr<Key> const& rootKey);
  ~ClientAuthentication();

  Variant authsvrRequest(String const& username, String const& passwordHash, String const& authClaim, Key const& clientPrivateKey);
  Variant authsvrResponse(String const& response, Key const& clientPrivateKey);
  String serverRequest();
  bool serverResponse(String const& response);

  Variant authsvrValidateRequest(Variant const& claim, String const& authClaim, Key const& clientPrivateKey);
  bool authsvrValidateResponse(String const& response, Key const& clientPrivateKey);

private:
  shared_ptr<Key> m_rootPublicKey;
};

}

}

#endif
