#include "StarAuthenticationService.hpp"
#include "StarAuthenticationKey.hpp"
#include "StarClock.hpp"
#include "StarLogging.hpp"
#include "StarFile.hpp"
#include <ctime>

namespace Star {
namespace Auth {

AuthenticationService::AuthenticationService(DatabasePtr db) {
  m_db = db;

  Variant config = Variant::parseJson(File::readFileString("auth.config"));

  m_rootPublicKey.loadPublicKey(config.getString("rootPublicKey"));
  m_authPrivateKey.loadPrivateKey(config.getString("authPrivateKey"));

  m_certificate = config.get("authCertificate");

  String certMetadata = config.get("authCertificate").get("signedAuthKey").repr(0, true);
  String certSignature = config.get("authCertificate").get("rootSignedAuthKeySignature").toString();
  if (!m_rootPublicKey.verifyMessage(certMetadata, certSignature)) {
    throw CryptoException("Invalid Certificate");
  }
}

AuthenticationService::~AuthenticationService() {
}

/**
 * {
 *    "signedAuthKey" : {
 *      "authPublicKey": ...
 *      "validFrom":...
 *      "validTo":...
 *    }
 *    "rootSignedAuthKeySignature": (signature made with root privkey)
 * }
 */
Variant AuthenticationService::getCertificate() {
  return m_certificate;
}

/**
 * Process Client Request
 *
 *  request:
 *  {
 *    "envelopeSignature" (signed with client privkey)
 *    "envelopeKey" (encyrpted with auth pubkey)
 *    "envelopeBody" === { encrypted body
 *      "request": {
 *        "metadata": {
 *          "username"
 *          "password"
 *        }
 *        "signature"
 *      }
 *      "claim" : {
 *        "metadata: {
 *          "clientPublicKey": (client public key)
 *          "username": (username)
 (          "validFrom": (utctime)
 *          "validTo": (utctime)
 *        },
 *        "signature": (produced by signing the 'claim' with the client private key)
 *      }
 *    }
 *  }
 *
 *  response:
 *  {
 *    "claim": {
 *      "metadata: {
 *        "clientPublicKey":...
 *        "username":...
 *        "validFrom":...
 *        "validTo":...
 *      }
 *      "signature"
 *    }
 *    "authSignature": {
 *      "signedAuthKey" : {
 *        "authPublicKey": ...
 *        "validFrom":...
 *        "validTo":...
 *      }
 *      "rootSignedAuthKeySignature": (signature made with root privkey)
 *    }
 *    "signature": (signature of main message with authPrivateKey)
 *  }
 *
 */
Variant AuthenticationService::authorizeClient(Variant const& request) {
  String envelopeSignature = request.getString("envelopeSignature");
  String envelopeBodyString = request.getString("envelopeBody");
  String envelopeKey = m_authPrivateKey.decryptMessage(request.getString("envelopeKey"));
  String envelopeBody = Star::Auth::decryptMessage(envelopeBodyString, envelopeKey);

  Variant requestMessage = Variant::parse(envelopeBody);

  Variant claim = requestMessage["claim"];

  if (!validateClientInnerClaim(claim))
    throw StarException("Request claim fail.");

  Key clientKey(claim["metadata"].getString("clientPublicKey"));

  if (!clientKey.verifyMessage(envelopeBodyString, envelopeSignature))
    throw StarException("Request outer signature fail.");

  String requestSignature = requestMessage["request"].getString("signature");
  auto requestMetadata = requestMessage["request"]["metadata"];
  if (!clientKey.verifyMessage(requestMetadata.repr(0, true), requestSignature))
    throw StarException("Request signature fail.");

  int64_t validFrom =  claim["metadata"].getInt("validFrom");
  int64_t validTo =  claim["metadata"].getInt("validTo");

  if (Clock::currentTime() - 1LL*25LL*60LL*60LL*1000LL > validFrom)
    throw StarException("Claim is too old.");

  if (Clock::currentTime() < validFrom)
    throw StarException("Claim not yet valid");

  if (Clock::currentTime()+ 8LL*24LL*60LL*60LL*1000LL < validTo)
    throw StarException("Claim is valid for too long.");

  String username = requestMetadata.getString("username");
  String password = requestMetadata.getString("password");
  String claimUsername = claim["metadata"].getString("username");

  if (username != claimUsername) {
    throw StarException("Username does not match claim.");
  }

  if (!m_db->validateUserAndPassword(username, password)) {
    throw StarException("Unable to authenticate user");
  }

  Variant responseMessage = VariantMap{
      {"claim", claim},
      {"authSignature", m_certificate},
      {"signature", m_authPrivateKey.signMessage(claim.repr(0, true))}
    };

  String messageKey = Star::Auth::generateKey();
  String responseBodyString = responseMessage.repr(0, true);
  String responseEnvelopeBody = encryptMessage(responseBodyString, messageKey);
  Variant responseEnvelope = VariantMap{
      {"envelopeBody", responseEnvelopeBody},
      {"envelopeKey", clientKey.encryptMessage(messageKey)},
      {"envelopeSignature", m_authPrivateKey.signMessage(responseEnvelopeBody)}
    };
  return responseEnvelope;
}

/**
 * Process Client Request
 *
 *  request:
 *  {
 *    "envelopeSignature" (signed with client privkey)
 *    "envelopeKey" (encyrpted with auth pubkey)
 *    "envelopeBody" === { crypted body
 *      "request": {
 *        "metadata": (claim)
 *        "signature"
 *      }
 *    }
 *  }
 *
 *  response:
 *  {
 *    "result": (bool)
 *    "authSignature": {
 *      "signedAuthKey" : {
 *        "authPublicKey": ...
 *        "validFrom":...
 *        "validTo":...
 *      }
 *      "rootSignedAuthKeySignature": (signature made with root privkey)
 *    }
 *  }
 *  // message is fullbody encrypted so the response is trust worthyish
 *
 */
Variant AuthenticationService::validateClient(Variant const& request) {
  bool valid = false;
  String clientPublicKey;
  try {
    String envelopeSignature = request.getString("envelopeSignature");
    String envelopeBodyString = request.getString("envelopeBody");
    String envelopeKey = m_authPrivateKey.decryptMessage(request.getString("envelopeKey"));
    String envelopeBody = Star::Auth::decryptMessage(envelopeBodyString, envelopeKey);

    Variant requestMessage = Variant::parse(envelopeBody);

    Variant claim = requestMessage["request"]["metadata"];

    clientPublicKey = claim["claim"]["metadata"].getString("clientPublicKey");

    if (!validateClientClaim(m_rootPublicKey, claim))
      throw StarException("Request claim fail.");

    Key clientKey(clientPublicKey);

    if (!clientKey.verifyMessage(envelopeBodyString, envelopeSignature))
      throw StarException("Request outer signature fail.");

    String requestSignature = requestMessage["request"].getString("signature");
    auto requestMetadata = requestMessage["request"]["metadata"];
    if (!clientKey.verifyMessage(requestMetadata.repr(0, true), requestSignature))
      throw StarException("Request signature fail.");

	  String username = claim["claim"]["metadata"].getString("username");

	  if (!m_db->validateUser(username)) {
	    throw StarException("Username is not valid.");
	  }
    valid = true;
  }
  catch (std::exception const& e) {
    Logger::error("failure validating client claim %s", e.what());
    valid = false;
  }

  Variant responseMessage = VariantMap{
      {"result", valid},
      {"authSignature", m_certificate}
    };

  String messageKey = Star::Auth::generateKey();
  String responseBodyString = responseMessage.repr(0, true);
  String envelopeBody = encryptMessage(responseBodyString, messageKey);
  Key clientKey(clientPublicKey);
  Variant responseEnvelope = VariantMap{
      {"envelopeBody", envelopeBody},
      {"envelopeKey", clientKey.encryptMessage(messageKey)},
      {"envelopeSignature", m_authPrivateKey.signMessage(envelopeBody)}
    };
  return responseEnvelope;
}


/**
 * response:
 * {
 *   "rootPublicKey" : string,
 *   "authPrivateKey" : string,
 *   "authCertificate" : {
 *     "signedAuthKey" : {
 *       "authPublicKey" : string,
 *       "validFrom" : long,
 *       "validTo" : long,
 *     }
 *     "rootSignedAuthKeySignature" : string,
 *   }
 * }
 *
 */
Variant AuthenticationService::generateAuthenticationConfig(String const& rootPrivateKey, int64_t valid, int64_t expires) {
  // Load Root Key
  Star::Auth::Key rootKey(rootPrivateKey, true);
  // Generate new Authsvr Key
  Star::Auth::Key authKey(true);

  Variant signedAuthKey = VariantMap{
      {"authPublicKey", authKey.publicKey()},
      {"validFrom", valid},
      {"validTo", expires}
    };

  Variant config = VariantMap{
      {"rootPublicKey", rootKey.publicKey()},
      {"authPrivateKey", authKey.privateKey()},
      {"authCertificate", VariantMap{
        {"signedAuthKey", signedAuthKey},
        {"rootSignedAuthKeySignature", rootKey.signMessage(signedAuthKey.repr(0, true))}
      }}
    };

  if (!validateAuthSignature(rootKey, config["authCertificate"]))
    throw StarException("Generation failed.");

  return config;
}

bool AuthenticationService::validateAuthSignature(Key const& rootPublicKey, Variant const& authSignature) {
  auto signedAuthKey = authSignature.get("signedAuthKey");
  auto rootSignedAuthKeySignature = authSignature.getString("rootSignedAuthKeySignature");
  if (!rootPublicKey.verifyMessage(signedAuthKey.repr(0, true), rootSignedAuthKeySignature)) {
    Logger::error("failed to validate signedAuthKey with rootSignedAuthKeySignature");
    return false;
  }

  auto authPublicKey = signedAuthKey.getString("authPublicKey");
  if (authPublicKey.empty()) {
    Logger::error("empty public key");
    return false;
  }
  auto signedAuthKeyValidFrom = signedAuthKey.getInt("validFrom");
  auto signedAuthKeyValidTo = signedAuthKey.getInt("validTo");

  if ((signedAuthKeyValidFrom > Clock::currentTime()) || (signedAuthKeyValidTo < Clock::currentTime())) {
    Logger::error("timestamp fail %s %s %s", Clock::currentTime(), signedAuthKeyValidFrom, signedAuthKeyValidTo);
    return false;
  }

  // implicit in use after this anyways
  //  Key authPublicKey(signedAuthKey);
  return true;
}

bool AuthenticationService::validateClientClaim(Key const& rootPublicKey, Variant const& claim) {
  if (!validateAuthSignature(rootPublicKey, claim.get("authSignature")))
    return false;

  auto signedAuthKey = claim.get("authSignature").get("signedAuthKey").getString("authPublicKey");
  Key authPublicKey(signedAuthKey);

  auto authSignature = claim.getString("signature");
  auto signedClaim = claim.get("claim");
  if (!authPublicKey.verifyMessage(signedClaim.repr(0, true), authSignature))
    return false;

  if (!validateClientInnerClaim(signedClaim))
    return false;

  return true;
}

bool AuthenticationService::validateClientInnerClaim(Variant const& claim) {
  auto claimMetadata = claim.get("metadata");
  auto clientPublicKeyData = claimMetadata.getString("clientPublicKey");
  auto claimSignature = claim.getString("signature");
  Key clientPublicKey(clientPublicKeyData);
  if (!clientPublicKey.verifyMessage(claimMetadata.repr(0, true), claimSignature))
    return false;

  auto validFrom = claimMetadata.getInt("validFrom");
  auto validTo = claimMetadata.getInt("validTo");

  if ((validFrom > Clock::currentTime()) || (validTo < Clock::currentTime()))
    return false;

  return true;
}

}
}

