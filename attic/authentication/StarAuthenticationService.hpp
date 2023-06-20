#ifndef _STAR_AUTHENTICATION_SERVICE_HPP_
#define _STAR_AUTHENTICATION_SERVICE_HPP_

#include "StarAuthenticationDatabaseFacade.hpp"
#include "StarAuthenticationKey.hpp"
#include "StarVariant.hpp"
#include "StarVariantExtra.hpp"

namespace Star {
namespace Auth {

class AuthenticationService {
public:
  static Variant generateAuthenticationConfig(String const& rootPrivateKey, int64_t valid, int64_t expires);
  static bool validateAuthSignature(Key const& rootPublicKey, Variant const& authSignature);
  static bool validateClientClaim(Key const& rootPublicKey, Variant const& claim);
  static bool validateClientInnerClaim(Variant const& claim);

  AuthenticationService(DatabasePtr db);
  ~AuthenticationService();

  Variant getCertificate();
  Variant authorizeClient(Variant const& request);
  Variant validateClient(Variant const& request);

private:
  Key m_authPrivateKey;
  Key m_rootPublicKey;
  DatabasePtr m_db;
  Variant m_certificate;
};


}
}

#endif
