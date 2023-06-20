#include "StarAuthenticationDatabase.hpp"
#include "StarLogging.hpp"
#include "StarDatabaseConnector.hpp"
#include "StarAlgorithm.hpp"

using namespace std;

namespace Star {
namespace Auth {


AuthenticationDatabase::AuthenticationDatabase(String const& connectionString) :m_connectionString(connectionString) {
  /*
  if (validateUser("none"))
    throw StarException("User none should not exist");
  if (!validateUser("test"))
    throw StarException("User test should exist");
  if (!validateUserAndPassword("test", preHashPassword("test", "test")))
    throw StarException("User test should be able to login using the password 'test'");
  if (validateUserAndPassword("test", preHashPassword("test", "none")))
    throw StarException("User test should not be able to login using the password 'none'");
    */
}

AuthenticationDatabase::~AuthenticationDatabase() {

}

bool AuthenticationDatabase::validateUser(String const& username) {
  DatabaseConnector conn(m_connectionString);
  {
    conn.exec("BEGIN");
    bool success = false;
    auto transactionScope = finally([&]() {
      if (success)
        conn.exec("END");
      else
        conn.exec("ROLLBACK");
    });
    auto rows = conn.fetch("SELECT id_user, active FROM users WHERE username=$1", { username });
    bool result;
    if (rows.size() == 0) {
      result = false;
    }
    else
    {
      starAssert(rows.size() == 1);
      auto row = rows[0];
      result = row.get("active").toBool();
    }
    conn.exec("INSERT INTO validations (username, timestamp, result) VALUES ($1, $2, $3)", { username, Thread::currentTime(), result });
    success = true;
    return result;
  }
}

bool AuthenticationDatabase::validateUserAndPassword(String const& username, String const& password) {
  DatabaseConnector conn(m_connectionString);
  {
    conn.exec("BEGIN");
    bool success = false;
    auto transactionScope = finally([&]() {
      if (success)
        conn.exec("END");
      else
        conn.exec("ROLLBACK");
    });
    auto rows = conn.fetch("SELECT id_user, active, salt, rounds, hash FROM users WHERE username=$1", { username });
    bool result;
    if (rows.size() == 0) {
      result = false;
    }
    else {
      starAssert(rows.size() == 1);
      auto row = rows[0];
      result = row.get("active").toBool();
      if (result)
        result = bcryptValidate(password, row.get("salt").toString(), row.get("hash").toString(), row.get("rounds").toInt());
    }

    conn.exec("INSERT INTO validations (username, timestamp, result) VALUES ($1, $2, $3)", { username, Thread::currentTime(), result });
    success = true;
    return result;
  }
}

bool AuthenticationDatabase::setUserRecord(String const& username, String const& passwordPreHashed, bool active) {
  DatabaseConnector conn(m_connectionString);
  {
    conn.exec("BEGIN");
    bool success = false;
    auto transactionScope = finally([&]() {
      if (success)
        conn.exec("END");
      else
        conn.exec("ROLLBACK");
    });
    String salt = generateKey();
    int rounds = 0;
    String hash = bcrypt(passwordPreHashed, salt, rounds);
    auto rows = conn.fetch("SELECT id_user FROM users WHERE username=$1", { username });
    if (rows.size() == 1) {
      conn.exec("UPDATE users SET salt=$2, rounds=$3, hash=$4, active=$5 WHERE id_user=$1", { rows[0].get("id_user"), salt, rounds, hash, active });
    }
    else {
      long since = Thread::currentTime();
      conn.exec("INSERT INTO users (username, salt, rounds, hash, since, active) VALUES ($1, $2, $3, $4, $5, $6)", { username, salt, rounds, hash, since, active });
    }
    success = true;
    return true;
  }
}

bool AuthenticationDatabase::activateUserRecord(String const& username, bool active) {
  DatabaseConnector conn(m_connectionString);
  {
    conn.exec("BEGIN");
    bool success = false;
    bool result;
    auto transactionScope = finally([&]() {
      if (success)
        conn.exec("END");
      else
        conn.exec("ROLLBACK");
    });
    auto rows = conn.fetch("SELECT id_user FROM users WHERE username=$1", { username });
    if (rows.size() == 1) {
      conn.exec("UPDATE users SET active=$2 WHERE id_user=$1", { rows[0].get("id_user"), active });
      result = true;
    }
    else
      result = false;
    success = true;
    return result;
  }
}

int AuthenticationDatabase::usersCount() {
  DatabaseConnector conn(m_connectionString);
  {
    conn.exec("BEGIN");
    int result = 0;
    bool success = false;
    auto transactionScope = finally([&]() {
      if (success)
        conn.exec("END");
      else
        conn.exec("ROLLBACK");
    });
    auto rows = conn.fetch("SELECT count(*) as amount FROM users", {});
    if (rows.size() == 1) {
      success = true;
      result = rows[0].get("amount").toInt();
    }
    return result;
  }
}

}
}
