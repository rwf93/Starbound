#include "StarServerShell.hpp"
#include "StarUniverseServer.hpp"
#include "StarLua.hpp"

namespace Star {

ServerShell::ServerShell(UniverseServerWeakPtr universe)
  : m_universe(universe) {}

void ServerShell::run() {
  
}

String ServerShell::help() const {
  return "";
}

UniverseServerPtr ServerShell::universe() const {
  if (auto universe = m_universe.lock()) {
    return universe;
  }

  throw ServerShellException("Running universe not valid.");
}

}
