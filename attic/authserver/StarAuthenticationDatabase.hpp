#ifndef _STAR_AUTHENTICATOR_DATABASE_HPP_
#define _STAR_AUTHENTICATOR_DATABASE_HPP_

#include "StarAuthenticationKey.hpp"
#include "StarAuthenticationService.hpp"
#include "StarException.hpp"
#include "StarVariant.hpp"
#include "StarRoot.hpp"
#include "StarTcp.hpp"
#include "StarThread.hpp"

namespace Star {

namespace Auth {

class AuthenticationDatabase: public Database {
public:
  AuthenticationDatabase(String const& connectionString);
  ~AuthenticationDatabase();
  virtual bool validateUser(String const& username) override;
  virtual bool validateUserAndPassword(String const& username, String const& passwordPreHashed) override;
  virtual bool setUserRecord(String const& username, String const& passwordPreHashed, bool active) override;
  virtual bool activateUserRecord(String const& username, bool active) override;
  virtual int usersCount() override;

private:
  String m_connectionString;
};

}

}

#endif
