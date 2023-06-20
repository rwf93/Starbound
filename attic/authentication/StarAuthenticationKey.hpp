#ifndef _STAR_AUTHENTICATION_KEY_HPP_
#define _STAR_AUTHENTICATION_KEY_HPP_

#include "StarException.hpp"
#include "StarVariant.hpp"
#include <openssl/evp.h>
#include <string>

namespace Star {

namespace Auth {

STAR_EXCEPTION(CryptoException, StarException);
STAR_EXCEPTION(AuthException, StarException);

class Key {
public:
  Key();
  Key(bool generate);
  Key(String const& key, bool privateKey = false);

  ~Key();
  void regenerate();
  void loadPrivateKey(String const& key);
  void loadPublicKey(String const& key);
  bool isPrivateKey() const {
    return m_isPrivate;
  }
  String publicKey() const;
  String privateKey() const;
  String encryptMessage(String const& message) const;
  String decryptMessage(String const& message) const;
  String signMessage(String const& message) const;
  bool verifyMessage(String const& message, String const& signature) const;

private:
  static int const KeySize = 4096;

  // Deleted copy constructor / assignment operator.
  Key(Key const& that);
  Key& operator=(Key const&);

  bool m_isPrivate;
  EVP_PKEY * m_key;
  EVP_MD_CTX * m_mdCtx;

  friend std::ostream &operator<<(std::ostream &cout, const Key &key);

  CryptoException cryptoException(EVP_PKEY_CTX *ctx, const char* errorString = nullptr) const;
  CryptoException cryptoException(BIO *bio, const char* errorString = nullptr) const;
};

void initializeKeyLogic();
String generateKey();
String encryptMessage(String const& message, String const& key);
String decryptMessage(String const& message, String const& key);

String preHashPassword(String const& username, String const& password);

String bcrypt(String const& message, String const& salt, int& rounds);
String bcryptWithRounds(String const& message, String const& salt, int rounds);
bool bcryptValidate(String const& message, String const& salt, String const& hash, int rounds);

}

}

#endif
