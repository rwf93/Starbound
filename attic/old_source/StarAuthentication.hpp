#ifndef STAR_AUTHENTICATION_HPP
#define STAR_AUTHENTICATION_HPP

#include "StarVariant.hpp"
#include "StarThread.hpp"

namespace Star {

STAR_CLASS(AuthenticationImpl); 
STAR_CLASS(SharedClaim); 
STAR_CLASS(Authentication); 

class SharedClaim {
public:
  virtual ~SharedClaim() {};
  virtual bool validateClaim() = 0;

  virtual Variant claim() = 0;
  virtual String encrypt(String const& message) = 0;
  virtual bool verify(String const& message, String const& signature) = 0;

  virtual String username() = 0;
};

class Authentication {
public:
  void load();

  bool validateClaim(bool remoteValidate);
  bool logon(String const& username, String const& passwordHash);

  Variant claim();

  String decrypt(String const& message);
  String sign(String const& message);

  SharedClaimPtr sharedClaim(Variant const& data);

private:
  AuthenticationImplPtr m_impl;
  Mutex m_mutex;
};

}

#endif
