#ifndef _STAR_AUTHENTICATION_DATABASE_HPP_
#define _STAR_AUTHENTICATION_DATABASE_HPP_

#include "StarException.hpp"
#include "StarVariant.hpp"

namespace Star {

namespace Auth {

class Database {
public:
  virtual ~Database() {}

  virtual bool validateUser(String const& username) = 0;
  virtual bool validateUserAndPassword(String const& username, String const& passwordPreHashed) = 0;
  virtual bool setUserRecord(String const& username, String const& passwordPreHashed, bool active) = 0;
  virtual bool activateUserRecord(String const& username, bool active) = 0;
  virtual int usersCount() = 0;
};
typedef shared_ptr<Database> DatabasePtr;

}

}

#endif
