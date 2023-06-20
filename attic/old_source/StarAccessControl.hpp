#ifndef STAR_ACCESS_CONTROL_HPP
#define STAR_ACCESS_CONTROL_HPP

#include "StarVariant.hpp"
#include "StarGameTypes.hpp"
#include "StarThread.hpp"
#include "StarSet.hpp"

namespace Star {

STAR_CLASS(UniverseServer);
STAR_CLASS(AccessControlClient);
STAR_CLASS(AccessControl);

class AccessControlClient {
public:
  AccessControlClient(AccessControl* control);

  bool setAccountName(String const& account);
  bool setPlayerName(String const& playerName);

  String passwordSalt();
  int passwordRounds();
  bool validate(String const& passwordHash);

  String processCommand(String const& command, StringList const& arguments);

private:
  AccessControl* m_control;
  String m_account;
  String m_playerName;
  String m_salty;
  int m_rounds;
  bool m_intialized;
};

class AccessControl {
public:
  AccessControl(String const& storageDirectory, UniverseServer* universeServer);

  pair<String, int> passwordSaltAndRounds(String const& account);
  bool validate(String const& account, String const& passwordHash, int rounds, String const& passwordSalt);

  bool isBannedAddress(String const& connectionString);
  bool isBannedAccountName(String const& account);
  bool isBannedPlayerName(String const& playerName);

  Variant accountValues(String const& account);

  AccessControlClientPtr clientConnected();

  String processCommand(String const& account, String const& command, StringList const& arguments);

private:
  UniverseServer* m_universeServer;
  String m_storageDirectory;
  Variant m_state;
  Mutex m_mutex;

  Set<String> m_bannedAddresses;
  Set<String> m_bannedAccountNames;
  Set<String> m_bannedPlayerNames;

  void load();
  void save();
};

}

#endif
