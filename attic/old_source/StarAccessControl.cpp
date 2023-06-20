#include "StarAccessControl.hpp"
#include "StarLogging.hpp"
#include "StarLexicalCast.hpp"
#include "StarFile.hpp"
#include "StarVariantExtra.hpp"
#include "StarAuthentication.hpp"
#include "StarAuthenticationKey.hpp"
#include "StarConfiguration.hpp"
#include "StarRoot.hpp"

namespace Star {

AccessControlClient::AccessControlClient(AccessControl* control) {
  m_control = control;
  m_rounds = 0;
  m_intialized = false;
}

bool AccessControlClient::setAccountName(String const& account) {
  // check if either the account  has been banned
  if (m_control->isBannedAccountName(account))
    return false;
  m_account = account;
  auto sr = m_control->passwordSaltAndRounds(m_account);
  m_salty = sr.first;
  m_rounds = sr.second;
  m_intialized = true;
  return true;
}

bool AccessControlClient::setPlayerName(String const& playerName) {
  if (m_control->isBannedPlayerName(playerName))
    return false;
  m_playerName = playerName;
  return true;
}

String AccessControlClient::passwordSalt() {
  if (!m_intialized)
    throw StarException();
  return m_salty;
}

int AccessControlClient::passwordRounds() {
  if (!m_intialized)
    throw StarException();
  return m_rounds;
}

bool AccessControlClient::validate(String const& passwordHash) {
  return m_control->validate(m_account, passwordHash, passwordRounds(), passwordSalt());
}

String AccessControlClient::processCommand(String const& command, StringList const& arguments) {
  return m_control->processCommand(m_account, command, arguments);
}

AccessControl::AccessControl(String const& storageDirectory, UniverseServer* universeServer) {
  m_universeServer = universeServer;
  m_storageDirectory = storageDirectory;
  load();
}

void AccessControl::load() {
  if (!File::isFile(m_storageDirectory + "/accesscontrol.config")) {
    Logger::warn("AccessControl: Failed to read accesscontrol file, creating a new file");
    VariantMap vm;
    vm.add("accounts", VariantMap());
    vm.add("bannedAddresses", VariantList());
    vm.add("bannedAccountNames", VariantList());
    vm.add("bannedPlayerNames", VariantList());
    {
      MutexLocker locked(m_mutex);
      m_state = vm;
    }
    save();
  }
  Variant newState = Variant::parseJson(File::readFileString(m_storageDirectory+"/accesscontrol.config"));
  MutexLocker locked(m_mutex);
  m_state = newState;

  m_bannedAddresses = variantToStringSet(m_state.get("bannedAddresses"));
  m_bannedAccountNames = variantToStringSet(m_state.get("bannedAccountNames"));
  m_bannedPlayerNames = variantToStringSet(m_state.get("bannedPlayerNames"));
}

void AccessControl::save() {
  MutexLocker locked(m_mutex);
  File::overwriteFileWithRename(m_state.repr(1, true), m_storageDirectory+"/accesscontrol.config", ".new");
}

bool AccessControl::isBannedAddress(String const& address) {
  MutexLocker locked(m_mutex);
  return m_bannedAddresses.contains(address);
}

bool AccessControl::isBannedAccountName(String const& account) {
  MutexLocker locked(m_mutex);
  return m_bannedAccountNames.contains(account);
}

bool AccessControl::isBannedPlayerName(String const& playerName) {
  MutexLocker locked(m_mutex);
  return m_bannedPlayerNames.contains(playerName);
}

AccessControlClientPtr AccessControl::clientConnected() {
  return make_shared<AccessControlClient>(this);
}

pair<String, int> AccessControl::passwordSaltAndRounds(String const& account) {
  _unused(account);
  return { Star::Auth::generateKey(), Root::singleton().configuration()->get("bcryptRounds").toInt()};
}

Variant AccessControl::accountValues(String const& account) {
  MutexLocker locked(m_mutex);
  if (m_state.get("accounts").contains(account))
    return m_state.get("accounts").get(account);
  return {};
}

bool AccessControl::validate(String const& account, String const& passwordHash, int rounds, String const& passwordSalt) {
  if (account.empty()) {
    for (auto password : Root::singleton().configuration()->get("serverPasswords").toList()) {
      // forward bcrypt validate, kind of cheating, normally you'd store the hashed variants
      if (Star::Auth::bcryptValidate(password.toString(), passwordSalt, passwordHash, rounds)) {
        return true;
      }
    }
  }
  else {
    auto accountSalt = account + passwordSalt;

    Variant accountEntry = accountValues(account);

    if (!accountEntry.isNull()) {
      auto password = accountEntry.getString("password");
      // forward bcrypt validate, kind of cheating, normally you'd store the hashed variants
      if (Star::Auth::bcryptValidate(password, accountSalt, passwordHash, rounds)) {
        return true;
      }
    }
  }
  return false;
}

String AccessControl::processCommand(String const& account, String const& command, StringList const& arguments) {
  _unused(account);
  _unused(arguments);
  // ac-help
  // ac-kick
  // ac-banhard
  // ac-banaccount
  // ac-banip
  // ac-players

  return strf("Command '%' not recognized", command);
}

}
