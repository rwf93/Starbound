#include "StarClientAuthentication.hpp"
#include "StarAuthenticationKey.hpp"
#include "StarAuthenticationService.hpp"
#include "StarAuthenticationConnection.hpp"
#include "StarClock.hpp"
#include "StarLogging.hpp"

#include <ctime>

namespace Star {
namespace Auth {

ClientAuthentication::ClientAuthentication(shared_ptr<Key> const& rootKey) {
  m_rootPublicKey = rootKey;
}

ClientAuthentication::~ClientAuthentication() {
}

/*
 *   {
 *     "envelopeKey"
 *     "envelopeBody" === { encrypted body
 *       "request": {
 *         "metadata": {
 *           "username"
 *           "password"
 *         }
 *         "signature"
 *       }
 *       "claim" : {
 *         "metadata: {
 *           "clientPublicKey": (client public key)
 *           "username": (username)
 *           "validFrom": (utctime)
 *           "validTo": (utctime)
 *         }
 *         "signature": (produced by signing the 'claim' with the client private key)
 *       }
 *     }
 *   }
 */

Variant ClientAuthentication::authsvrRequest(String const& username, String const& passwordHash, String const& authClaim, Key const& clientPrivateKey) {
  Variant authClaimSignature = Variant::parse(authClaim);

  if (!AuthenticationService::validateAuthSignature(*m_rootPublicKey, authClaimSignature))
    throw StarException("Auth claim fail.");

  Key authPublicKey(authClaimSignature.get("signedAuthKey").getString("authPublicKey"));

  Variant requestMetadata = VariantMap{
      make_pair("username", username),
      make_pair("password", passwordHash)
    };

  Variant claimMetadata = VariantMap{
      make_pair("clientPublicKey", clientPrivateKey.publicKey()),
      make_pair("username", username),
      make_pair("validFrom", Clock::currentTime() - 1LL * 1LL * 60LL * 60LL * 1000LL),
      make_pair("validTo", Clock::currentTime() + 7LL * 24LL * 60LL * 60LL * 1000LL)
    };

  Variant requestMessage = VariantMap{
      make_pair("request", VariantMap{
        make_pair("metadata", requestMetadata),
        make_pair("signature", clientPrivateKey.signMessage(requestMetadata.repr(0, true)))
      }),

      make_pair("claim", VariantMap{
        make_pair("metadata", claimMetadata),
        make_pair("signature", clientPrivateKey.signMessage(claimMetadata.repr(0, true)))
      })
    };

  String envelopeKey = Auth::generateKey();
  String requestEnvelopeBody = requestMessage.repr(0, true);
  String envelopeBody = Auth::encryptMessage(requestEnvelopeBody, envelopeKey);
  Variant requestEnvelope = VariantMap{
      make_pair("envelopeBody", envelopeBody),
      make_pair("envelopeKey", authPublicKey.encryptMessage(envelopeKey)),
      make_pair("envelopeSignature", clientPrivateKey.signMessage(envelopeBody)),
    };

  return requestEnvelope;
}

/**
 * Client claim format:
 *
 *  {
 *    "signedClaim" : {
 *      "claim: {
 *        "clientPublicKey": (client public key)
 *        "username": (username)
 *        "validFrom": (utctime)
 *        "validTo": (utctime)
 *      },
 *      "signature": (produced by signing the 'claim' with the client private key)
 *    },
 *    "authSignature" : {
 *      "signature" : (produced by signing the 'signedClaim' with the auth private key)
 *      "signedAuthKey": {
 *        "authPublicKey": (auth public key)
 *        "validFrom": (utctime)
 *        "validTo": (utctime)
 *      }
 *      "signedAuthKeySignature": (produced by signing the auth public key with the root private key)
 *    }
 *  }
 */

Variant ClientAuthentication::authsvrResponse(String const& response, Key const& clientPrivateKey) {
  Variant  requestEnvelope = Variant::parse(response);

  String envelopeKey = clientPrivateKey.decryptMessage(requestEnvelope.getString("envelopeKey"));
  String requestEnvelopeBody = requestEnvelope.getString("envelopeBody");
  String envelopeBody = Auth::decryptMessage(requestEnvelopeBody, envelopeKey);
  String envelopeSignature = requestEnvelope.getString("envelopeSignature");

  Variant authClaimSignature = Variant::parse(envelopeBody);

  Key authPublicKey(authClaimSignature.get("authSignature").get("signedAuthKey").getString("authPublicKey"));

  if (!authPublicKey.verifyMessage(requestEnvelopeBody, envelopeSignature))
    throw StarException("Envelop signature fail");

  return authClaimSignature;
}

String ClientAuthentication::serverRequest() {
  return "";
}

bool ClientAuthentication::serverResponse(String const& response) {
  //todo
  _unused(response);
  return true;
}

Variant ClientAuthentication::authsvrValidateRequest(Variant const& claim, String const& authClaim, Key const& clientPrivateKey) {
  Variant authClaimSignature = Variant::parse(authClaim);

  if (!AuthenticationService::validateAuthSignature(*m_rootPublicKey, authClaimSignature))
    throw StarException("Auth claim fail.");

  Key authPublicKey(authClaimSignature.get("signedAuthKey").getString("authPublicKey"));

  Variant requestMessage = VariantMap{
      {"request", VariantMap{
        {"metadata", claim},
        {"signature", clientPrivateKey.signMessage(claim.repr(0, true))}
      }}
    };

  String envelopeKey = Auth::generateKey();
  String requestEnvelopeBody = requestMessage.repr(0, true);
  String envelopeBody = Auth::encryptMessage(requestEnvelopeBody, envelopeKey);

  Variant requestEnvelope = VariantMap{
      {"envelopeBody", envelopeBody},
      {"envelopeKey", authPublicKey.encryptMessage(envelopeKey)},
      {"envelopeSignature", clientPrivateKey.signMessage(envelopeBody)}
    };

  return requestEnvelope;
}

bool ClientAuthentication::authsvrValidateResponse(String const& response, Key const& clientPrivateKey) {
  Variant requestEnvelope = Variant::parse(response);

  String envelopeKey = clientPrivateKey.decryptMessage(requestEnvelope.getString("envelopeKey"));
  String requestEnvelopeBody = requestEnvelope.getString("envelopeBody");
  String envelopeBody = Auth::decryptMessage(requestEnvelopeBody, envelopeKey);
  String envelopeSignature = requestEnvelope.getString("envelopeSignature");

  Variant authClaimSignature = Variant::parse(envelopeBody);

  Key authPublicKey(authClaimSignature.get("authSignature").get("signedAuthKey").getString("authPublicKey"));

  if (!authPublicKey.verifyMessage(requestEnvelopeBody, envelopeSignature))
    throw StarException("Envelop signature fail");

  return authClaimSignature.getBool("result");
}


}
}
