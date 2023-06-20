#ifndef STAR_SERVER_SHELL_HPP
#define STAR_SERVER_SHELL_HPP

#include "StarException.hpp"

namespace Star {

STAR_EXCEPTION(ServerShellException, StarException);

STAR_CLASS(UniverseServer);
STAR_CLASS(LuaContext);

class ServerShell {
public:
  ServerShell(UniverseServerWeakPtr universe);
  void run();

private:
  String help() const;
  UniverseServerPtr universe() const;

  UniverseServerWeakPtr m_universe;
  LuaContextPtr m_shellContext;
};

}

#endif
