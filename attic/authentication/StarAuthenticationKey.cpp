#include "StarAuthenticationKey.hpp"
#include "StarClock.hpp"
#include "StarEncode.hpp"
#include "StarSha256.hpp"
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <openssl/buffer.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <openssl/x509.h>
#include <vector>
#include "StarRandom.hpp"

namespace Star {

namespace Auth {

std::ostream &operator<<(std::ostream &out, const Key &key) {
  return out << "Private Key :" << key.privateKey() << std::endl << " Public Key :" << key.publicKey() << std::endl;
}

Key::Key() : m_key(NULL), m_mdCtx(NULL) {
  // ERR_load_crypto_strings will only load things once even if called multiple times
  ERR_load_crypto_strings();
  m_key = EVP_PKEY_new();
  if (m_key == NULL)
    throw CryptoException("Unable to Establish Key Container");
  m_mdCtx = EVP_MD_CTX_create();
  if (m_mdCtx == NULL) {
    EVP_PKEY_free(m_key);
    m_key = NULL;
    throw CryptoException("Unable to Establish EVP Message Digest Context");
  }
}

Key::Key(bool generate) : Key() {
  if (generate) {
    regenerate();
  }
}

Key::Key(String const& key, bool privateKey) : Key() {
  if (privateKey) {
    loadPrivateKey(key);
  } else {
    loadPublicKey(key);
  }
}

Key::~Key() {
  if (m_key != NULL)
    EVP_PKEY_free(m_key);
  if (m_mdCtx != NULL)
    EVP_MD_CTX_destroy(m_mdCtx);
  // We could call ERR_free_strings but that would just mean the next Key
  // to be created would make ERR_load_crypto_strings actually process
  // so eliminate that thrashing by not calling ERR_free_strings
}

void Key::regenerate() {
  EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
  if (EVP_PKEY_keygen_init(ctx) <= 0)
    throw cryptoException(ctx, "Unable to initialize EVP Key Generator");
  if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, KeySize) <= 0)
    throw cryptoException(ctx, "Unable to set EVP Key Generator size");
  if (EVP_PKEY_keygen(ctx, &m_key) <= 0)
    throw cryptoException(ctx, "Unable to generate new Client Key");
  EVP_PKEY_CTX_free(ctx);
  m_isPrivate = 1;
}

CryptoException Key::cryptoException(BIO *bio, const char* errorString) const {
  unsigned long err = ERR_get_error();
  BIO_free_all(bio);
  if (errorString == nullptr) {
    errorString = ERR_error_string(err, NULL);
  }
  return CryptoException(ERR_error_string(err, NULL));
}

CryptoException Key::cryptoException(EVP_PKEY_CTX *ctx, const char* errorString) const {
  unsigned long err = ERR_get_error();
  EVP_PKEY_CTX_free(ctx);
  if (errorString == nullptr) {
    errorString = ERR_error_string(err, NULL);
  }
  return CryptoException(errorString);
}

void Key::loadPrivateKey(String const& key) {
  if (key.empty())
    throw StarException("Empty key.");
  BIO *bIn = BIO_push(
      BIO_new(BIO_f_base64()),
      BIO_new_mem_buf((void*) key.utf8Ptr(), key.utf8Size())
          );
  BIO_set_flags(bIn, BIO_FLAGS_BASE64_NO_NL);

  RSA * rsa;
  if ((rsa = d2i_RSAPrivateKey_bio(bIn, NULL)) == 0)
    throw cryptoException(bIn);

  EVP_PKEY_set1_RSA(m_key, rsa);
  BIO_free_all(bIn);
  m_isPrivate = 1;
}

void Key::loadPublicKey(String const& key) {
  if (key.empty())
    throw StarException("Empty key.");
  BIO *bIn = BIO_push(
      BIO_new(BIO_f_base64()),
      BIO_new_mem_buf((void*) key.utf8Ptr(), key.utf8Size())
          );
  BIO_set_flags(bIn, BIO_FLAGS_BASE64_NO_NL);

  RSA * rsa;
  if ((rsa = d2i_RSAPublicKey_bio(bIn, NULL)) == 0)
    throw cryptoException(bIn);

  EVP_PKEY_set1_RSA(m_key, rsa);
  BIO_free_all(bIn);
  m_isPrivate = 0;
}

String Key::publicKey() const {
  BIO *b64 = BIO_push(
      BIO_new(BIO_f_base64()),
      BIO_new(BIO_s_mem())
          );
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

  i2d_RSAPublicKey_bio(b64, EVP_PKEY_get1_RSA(m_key));
  (void) BIO_flush(b64);

  BUF_MEM *bPtr;
  BIO_get_mem_ptr(b64, &bPtr);
  String out(bPtr->data, bPtr->length);
  BIO_free_all(b64);

  return out;
}

String Key::privateKey() const {
  if (!m_isPrivate)
    throw CryptoException("Private Key not loaded.");
  BIO *b64 = BIO_push(
      BIO_new(BIO_f_base64()),
      BIO_new(BIO_s_mem())
          );
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

  i2d_RSAPrivateKey_bio(b64, EVP_PKEY_get1_RSA(m_key));
  (void) BIO_flush(b64);

  BUF_MEM *bPtr;
  BIO_get_mem_ptr(b64, &bPtr);
  String out(bPtr->data, bPtr->length);
  BIO_free_all(b64);

  return out;
}

String Key::encryptMessage(String const& message) const {
  size_t bufLen;
  unsigned char * buf;

  EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(m_key, NULL);
  if (EVP_PKEY_encrypt_init(ctx) != 1)
    throw cryptoException(ctx);

  if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) != 1)
    throw cryptoException(ctx);

  if (EVP_PKEY_encrypt(ctx, NULL, &bufLen, (unsigned char *) message.utf8Ptr(), message.utf8Size()) != 1)
    throw cryptoException(ctx);

  buf = new unsigned char[bufLen];
  if (EVP_PKEY_encrypt(ctx, (unsigned char *) buf, &bufLen, (unsigned char *) message.utf8Ptr(), message.utf8Size()) <= 0) {
    delete[] buf;
    throw cryptoException(ctx);
  }
  EVP_PKEY_CTX_free(ctx);

  String oString(base64Encode((char const*)buf, bufLen));
  delete[] buf;
  return oString;
}

String Key::decryptMessage(String const& cryptText) const {
  ByteArray iString(base64Decode(cryptText));
  size_t bufLen;
  unsigned char * buf;
  EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(m_key, NULL);
  if (EVP_PKEY_decrypt_init(ctx) <= 0)
    throw cryptoException(ctx);
  if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
    throw cryptoException(ctx);
  if (EVP_PKEY_decrypt(ctx, NULL, &bufLen, (unsigned char *) iString.ptr(), iString.size()) <= 0)
    throw cryptoException(ctx);
  buf = new unsigned char[bufLen];
  if (EVP_PKEY_decrypt(ctx, (unsigned char *) buf, &bufLen, (unsigned char *) iString.ptr(), iString.size()) != 1) {
    delete[] buf;
    throw cryptoException(ctx);
  }
  EVP_PKEY_CTX_free(ctx);
  String oString((const char *) buf, bufLen);
  delete[] buf;
  return oString;
}

String Key::signMessage(String const& message) const {
  if (!m_isPrivate)
    throw CryptoException("Private Key not Loaded");
  size_t sigLen;
  unsigned char *sig;
  ByteArray shaMessage(sha256(ByteArray(message.utf8Ptr(), message.utf8Size())));
  EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(m_key, NULL);
  if (EVP_PKEY_sign_init(ctx) <= 0)
    throw cryptoException(ctx);
  if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PSS_PADDING) <= 0)
    throw cryptoException(ctx);
  if (EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256()) <= 0)
    throw cryptoException(ctx);
  if (EVP_PKEY_sign(ctx, NULL, &sigLen, (unsigned char *) shaMessage.ptr(), shaMessage.size()) <= 0)
    throw cryptoException(ctx);
  if (!(sig = (unsigned char *) OPENSSL_malloc(sigLen)))
    throw cryptoException(ctx);
  if (EVP_PKEY_sign(ctx, sig, &sigLen, (unsigned char *) shaMessage.ptr(), shaMessage.size()) <= 0)
    throw cryptoException(ctx);
  EVP_PKEY_CTX_free(ctx);
  String oString(base64Encode((const char *)sig, sigLen));
  OPENSSL_free(sig);
  return oString;
}

bool Key::verifyMessage(String const& message, String const& signature) const {
  ByteArray shaMessage(sha256(ByteArray(message.utf8Ptr(), message.utf8Size())));
  ByteArray rawSignature(base64Decode(signature));
  EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(m_key, NULL);
  if (EVP_PKEY_verify_init(ctx) <= 0)
    throw cryptoException(ctx);
  if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PSS_PADDING) <= 0)
    throw cryptoException(ctx);
  if (EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256()) <= 0)
    throw cryptoException(ctx);
  int rv = EVP_PKEY_verify(ctx, (unsigned char *) rawSignature.ptr(), rawSignature.size(), (unsigned char *) shaMessage.ptr(), shaMessage.size());
  if (rv < 0)
    throw cryptoException(ctx);
  EVP_PKEY_CTX_free(ctx);
  return rv == 1;
}

void initializeKeyLogic() {
  // intitial initialization of DES_random_key and friends scans the whole heap.
  // do it as early as possible.
  generateKey();
}

String generateKey() {
  // Init desKey to week key to avoid valgrind warning
  DES_cblock desKey = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
  ByteArray key = ByteArray::withReserve(24);

  if (DES_random_key(&desKey) == 0)
    throw CryptoException("RNG could not generate a secure key");
  key.append((const char *) desKey, 8);
  if (DES_random_key(&desKey) == 0)
    throw CryptoException("RNG could not generate a secure key");
  key.append((const char *) desKey, 8);
  if (DES_random_key(&desKey) == 0)
    throw CryptoException("RNG could not generate a secure key");
  key.append((const char *) desKey, 8);
  return base64Encode(key);
}

/**
 * Encrypts a Message
 * @param message Message to Encrypt
 * @param key Base64 encoded 24 byte key
 * @return Base64 Encoded String containing encrypted message
 */
String encryptMessage(String const& message, String const& key) {
  DES_cblock desKey;
  DES_cblock ivec;
  DES_key_schedule desSchedule1;
  DES_key_schedule desSchedule2;
  DES_key_schedule desSchedule3;
  int n = 0;

  /* Prepare the key for use with DES_cfb64_encrypt */
  ByteArray iKey(base64Decode(key));
  if (iKey.size() != 24)
    throw StarException("Key size mismatch.");

  std::memcpy(desKey, iKey.ptr(), 8);
  DES_set_odd_parity(&desKey);
  DES_set_key_checked(&desKey, &desSchedule1);
  std::memcpy(desKey, iKey.ptr() + 8, 8);
  DES_set_odd_parity(&desKey);
  DES_set_key_checked(&desKey, &desSchedule2);
  std::memcpy(desKey, iKey.ptr() + 16, 8);
  DES_set_odd_parity(&desKey);
  DES_set_key_checked(&desKey, &desSchedule3);

  std::memcpy(ivec, iKey.ptr() + 12, 8);

  /* Encryption occurs here */
  unsigned char * buf = new unsigned char[message.utf8Size()];
  DES_ede3_cfb64_encrypt((unsigned char *) message.utf8Ptr(), buf, message.utf8Size(), &desSchedule1, &desSchedule2, &desSchedule3, &ivec, &n, DES_ENCRYPT);
  String output(base64Encode((const char*)buf, message.utf8Size()));
  delete[] buf;
  return output;
}

/**
 * Decrypts a Message
 * @param message Base64 Encoded DES3 encrypted message
 * @param key Base64 encoded 24 byte key
 * @return String containing decrypted message
 */
String decryptMessage(String const& message, String const& key) {
  DES_cblock desKey;
  DES_cblock ivec;
  DES_key_schedule desSchedule1;
  DES_key_schedule desSchedule2;
  DES_key_schedule desSchedule3;
  int n = 0;

  ByteArray iMessage(base64Decode(message));

  /* Prepare the key for use with DES_cfb64_encrypt */
  ByteArray iKey(base64Decode(key));
  if (iKey.size() != 24)
    throw StarException("Key size mismatch.");

  std::memcpy(desKey, iKey.ptr(), 8);
  DES_set_odd_parity(&desKey);
  DES_set_key_checked(&desKey, &desSchedule1);
  std::memcpy(desKey, iKey.ptr() + 8, 8);
  DES_set_odd_parity(&desKey);
  DES_set_key_checked(&desKey, &desSchedule2);
  std::memcpy(desKey, iKey.ptr() + 16, 8);
  DES_set_odd_parity(&desKey);
  DES_set_key_checked(&desKey, &desSchedule3);

  std::memcpy(ivec, iKey.ptr() + 12, 8);

  /* Encryption occurs here */
  unsigned char * buf = new unsigned char[message.utf8Size()];
  DES_ede3_cfb64_encrypt((unsigned char *) iMessage.ptr(), buf, iMessage.size(), &desSchedule1, &desSchedule2, &desSchedule3, &ivec, &n, DES_DECRYPT);
  String output((const char *) buf, iMessage.size());
  delete[] buf;
  return output;
}

String preHashPassword(String const& username, String const& password) {
  String passPreHash = username + ":" + password + ":starbound";
  return base64Encode(sha256(passPreHash.utf8Ptr(), passPreHash.utf8Size()));
}

double const bcrypt_goal_duration = 0.032;
double const bcrypt_max_validate_duration = 0.5;
double const bcrypt_max_generation_duration = 5.0;
int const bcrypt_minimal_rounds = 100;

String bcrypt(String const& message, String const& salt, int& rounds) {
  rounds = 0;

  auto startTime = Clock::currentTime();

  ByteArray saltBuffer(salt.utf8Ptr(), salt.utf8Size());
  ByteArray messageBuffer(sha256(ByteArray(message.utf8Ptr(), message.utf8Size())));
  ByteArray roundBuffer;

  while ((rounds < bcrypt_minimal_rounds) || (Clock::secondsSince(startTime) < bcrypt_goal_duration)) {
    roundBuffer.resize(0);
    roundBuffer.writeFrom(messageBuffer.ptr(), 0, messageBuffer.size());
    roundBuffer.writeFrom(saltBuffer.ptr(), messageBuffer.size(), saltBuffer.size());
    sha256(roundBuffer, messageBuffer);
    rounds++;
  }
  return base64Encode(messageBuffer);
}

String bcryptWithRounds(String const& message, String const& salt, int rounds) {
  if (rounds < bcrypt_minimal_rounds)
    throw StarException("Not enough rounds for bcrypt.");
  ByteArray saltBuffer(salt.utf8Ptr(), salt.utf8Size());
  ByteArray messageBuffer(sha256(ByteArray(message.utf8Ptr(), message.utf8Size())));
  ByteArray roundBuffer;

  auto startTime = Clock::currentTime();

  while (rounds > 0) {
    if (Clock::secondsSince(startTime) > bcrypt_max_generation_duration)
      throw StarException("Timeout generating bcrypt.");
    roundBuffer.resize(0);
    roundBuffer.writeFrom(messageBuffer.ptr(), 0, messageBuffer.size());
    roundBuffer.writeFrom(saltBuffer.ptr(), messageBuffer.size(), saltBuffer.size());
    sha256(roundBuffer, messageBuffer);
    rounds--;
  }
  return base64Encode(messageBuffer);
}

bool bcryptValidate(String const& message, String const& salt, String const& hash, int rounds) {
  if (rounds < bcrypt_minimal_rounds)
    return false;
  ByteArray saltBuffer(salt.utf8Ptr(), salt.utf8Size());
  ByteArray messageBuffer(sha256(ByteArray(message.utf8Ptr(), message.utf8Size())));
  ByteArray roundBuffer;

  auto startTime = Clock::currentTime();

  while (rounds > 0) {
    if (Clock::secondsSince(startTime) > bcrypt_max_validate_duration)
      return false;
    roundBuffer.resize(0);
    roundBuffer.writeFrom(messageBuffer.ptr(), 0, messageBuffer.size());
    roundBuffer.writeFrom(saltBuffer.ptr(), messageBuffer.size(), saltBuffer.size());
    sha256(roundBuffer, messageBuffer);
    rounds--;
  }
  return base64Encode(messageBuffer) == hash;
}

}

}
