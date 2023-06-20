#include "StarAuthentication.hpp"
#include "StarRoot.hpp"
#include "StarConfiguration.hpp"
#include "StarClientAuthentication.hpp"
#include "StarAuthenticationConnection.hpp"
#include "StarAuthenticationService.hpp"
#include "StarLogging.hpp"
#include "StarFile.hpp"

using namespace Star::Auth;

namespace Star {

class AuthenticationImpl {
public:
  AuthenticationImpl(Configuration const* configuration) {
    m_rootKey = make_shared<Key>();
    m_clientKey = make_shared<Key>();
    m_rootKey->loadPublicKey(configuration->getString("rootKey"));
    m_authenticator = make_shared<ClientAuthentication>(m_rootKey);
    m_connector = make_shared<AuthenticationConnection>(configuration->getString("authHostname"), configuration->getInt("authPort"));
    m_claimFile = Root::singleton().toStoragePath(configuration->getString("claimFile"));
    m_hasClaim = false;
    m_lazyLoadClaim = true;
    m_remoteValidated = false;
  }

  bool validateClaim(bool remoteValidate) {
    if (m_lazyLoadClaim)
      loadClaim();
    if (validClaim()) {
      if (remoteValidate && !m_remoteValidated) {
        remoteValidateClaim();
      }
    }
    if (!m_hasClaim) {
      deleteClaim();
    }
    return m_hasClaim;
  }

  Variant claim() {
    if (!m_hasClaim)
      throw StarException("No valid claim.");
    return m_claim;
  }

  String decrypt(String const& message) {
    if (!m_hasClaim)
      throw StarException("No valid claim.");
    return m_clientKey->decryptMessage(message);
  }

  String sign(String const& message) {
    if (!m_hasClaim)
      throw StarException("No valid claim.");
    return m_clientKey->signMessage(message);
  }

  bool logon(String const& username, String const& passwordHash) {
    m_lazyLoadClaim = false;
    m_hasClaim = false;
    m_remoteValidated = false;
    try {
      Variant authPubKeyRequest = VariantMap{
          {"command", "getAuthKey"}
        };
      auto authSignatureString = m_connector->query(authPubKeyRequest.repr(0, true));
      Variant authSignature = Variant::parse(authSignatureString);
      if (!AuthenticationService::validateAuthSignature(*m_rootKey, authSignature))
        throw StarException("Auth server key failure.");

      m_clientKey->regenerate();

      auto authsvrRequest = m_authenticator->authsvrRequest(username, passwordHash, authSignatureString, *m_clientKey);

      Variant authorizeClientRequest = VariantMap{
          {"command", "authorizeClient"},
          {"body", authsvrRequest}
        };
      auto authsvrResponse = m_connector->query(authorizeClientRequest.repr(0, true));
      auto logonResult = m_authenticator->authsvrResponse(authsvrResponse, *m_clientKey);

      if (!AuthenticationService::validateClientClaim(*m_rootKey, logonResult))
        throw StarException("Logon failure.");

      m_claim = logonResult;
      m_hasClaim = true;
      m_remoteValidated = true;

      saveClaim();
    } catch (std::exception const& e) {
      Logger::error("Exception while validating claim. %s", e.what());
    }
    return m_hasClaim;
  }

  bool remoteValidateClaim() {
    if (!m_hasClaim)
      return m_hasClaim;
    try {
      Variant authPubKeyRequest = VariantMap{
          {"command", "getAuthKey"}
        };
      auto authSignatureString = m_connector->query(authPubKeyRequest.repr(0, true));
      Variant authSignature = Variant::parse(authSignatureString);
      if (!AuthenticationService::validateAuthSignature(*m_rootKey, authSignature))
        throw StarException("Auth server key failure.");

      auto authsvrRequest = m_authenticator->authsvrValidateRequest(m_claim, authSignatureString, *m_clientKey);
      Variant authorizeClientRequest = VariantMap{
          {"command", "validateClient"},
          {"body", authsvrRequest}
        };
      auto authsvrResponse = m_connector->query(authorizeClientRequest.repr(0, true));
      auto validateResult = m_authenticator->authsvrValidateResponse(authsvrResponse, *m_clientKey);
      if (!validateResult) {
        m_hasClaim = false;
        m_remoteValidated = false;
      }
    } catch (std::exception const& e) {
      // validation failure will not cause an exception btw
      Logger::error("Exception while validating claim. %s", e.what());
    }
    return m_hasClaim;
  }

  bool validClaim() {
    if (!m_hasClaim)
      return false;
    m_hasClaim = AuthenticationService::validateClientClaim(*m_rootKey, m_claim);
    if (!m_hasClaim)
      m_remoteValidated = false;
    return m_hasClaim;
  }

  void loadClaim() {
    m_lazyLoadClaim = false;
    try {
      m_hasClaim = false;
      m_remoteValidated = false;
      if (m_claimFile.empty())
        return;
      if (!File::isFile(m_claimFile))
        return;
      VariantMap file = Variant::parseJson(File::readFileString(m_claimFile)).toMap();
      m_claim = file["claim"];
      m_hasClaim = true;
      m_clientKey->loadPrivateKey(file["key"].toString());
      m_hasClaim = validClaim();
    } catch (std::exception const& e) {
      Logger::error("Exception while loading claim. %s", e.what());
    }
    if (!m_hasClaim) {
      deleteClaim();
    }
  }

  void saveClaim() {
    m_lazyLoadClaim = false;
    m_hasClaim = validClaim();
    if (!m_hasClaim) {
      deleteClaim();
      return;
    }
    if (m_claimFile.empty())
      return;
    try {
      VariantMap file;
      file.insert("key", m_clientKey->privateKey());
      file.insert("claim", m_claim);
      File::overwriteFileWithRename(Variant(file).toJson(1), m_claimFile);
    } catch (std::exception const& e) {
      Logger::error("Exception while saving claim. %s", e.what());
    }
  }

  void deleteClaim() {
    m_lazyLoadClaim = false;
    m_hasClaim = false;
    m_remoteValidated = false;
    m_claim = {};
    try {
      if (m_claimFile.empty())
        return;
      if (File::isFile(m_claimFile)) {
        if (File::isFile(m_claimFile + ".old"))
          File::remove(m_claimFile + ".old");
        File::rename(m_claimFile, m_claimFile + ".old");
        File::remove(m_claimFile + ".old");
      }
    } catch (std::exception const& e) {
      Logger::error("Exception while removing claim. %s", e.what());
    }
  }

  shared_ptr<ClientAuthentication> m_authenticator;
  shared_ptr<AuthenticationConnection> m_connector;
  String m_claimFile;
  bool m_lazyLoadClaim;
  bool m_hasClaim;
  bool m_remoteValidated;
  Variant m_claim;
  shared_ptr<Key> m_rootKey;
  shared_ptr<Key> m_clientKey;
};

class SharedClaimImpl: public SharedClaim {
public:
  SharedClaimImpl(AuthenticationImpl* parent, Variant const& claim) {
    m_parent = parent;
    m_claim = claim;
    m_clientKey = make_shared<Key>();
    m_clientKey->loadPublicKey(m_claim.get("claim").get("metadata").get("clientPublicKey").toString());
  }

  bool validateClaim() {
    return AuthenticationService::validateClientClaim(*(m_parent->m_rootKey), m_claim);
  }

  Variant claim() {
    return m_claim;
  }

  String encrypt(String const& message) {
    return m_clientKey->encryptMessage(message);
  }

  bool verify(String const& message, String const& signature) {
    return m_clientKey->verifyMessage(message, signature);
  }

  String username() {
    return m_claim.get("claim").get("metadata").getString("username");
  }

  AuthenticationImpl* m_parent;
  Variant m_claim;
  shared_ptr<Key> m_clientKey;
};

void Authentication::load() {
  MutexLocker locker(m_mutex);
  if (!m_impl)
    m_impl = make_shared<AuthenticationImpl>(Root::singleton().configuration());
}

bool Authentication::validateClaim(bool remoteValidate) {
  if (remoteValidate) {
    auto remotevalidationimpl = make_shared<AuthenticationImpl>(Root::singleton().configuration());
    bool result = remotevalidationimpl->validateClaim(true);
    {
      MutexLocker locker(m_mutex);
      m_impl = remotevalidationimpl;
    }
    return result;
  } else {
    MutexLocker locker(m_mutex);
    return m_impl->validateClaim(false);
  }
}

bool Authentication::logon(String const& username, String const& passwordHash) {
  MutexLocker locker(m_mutex);
  return m_impl->logon(username, passwordHash);
}

Variant Authentication::claim() {
  MutexLocker locker(m_mutex);
  return m_impl->claim();
}

String Authentication::decrypt(String const& message) {
  MutexLocker locker(m_mutex);
  return m_impl->decrypt(message);
}

String Authentication::sign(String const& message) {
  MutexLocker locker(m_mutex);
  return m_impl->sign(message);
}

SharedClaimPtr Authentication::sharedClaim(Variant const& data) {
  MutexLocker locker(m_mutex);
  if (!m_impl)
    throw StarException("Invalid state");
  return make_shared<SharedClaimImpl>(m_impl.get(), data);
}

}
