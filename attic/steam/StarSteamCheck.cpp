#include "StarSteamCheck.hpp"

#include "StarDynamicLib.hpp"
#include "StarLogging.hpp"

namespace Star {

SteamCheck::SteamCheck() {

}

typedef bool (__cdecl *SteamAPI_RestartAppIfNecessary_t)(uint32_t unOwnAppID);

bool SteamCheck::check() {
  auto lib = DynamicLib::loadLibrary("steam_api.dll");
  SteamAPI_RestartAppIfNecessary_t restartAppIfNecessary = (SteamAPI_RestartAppIfNecessary_t)lib->funcPtr("SteamAPI_RestartAppIfNecessary");
  if (!restartAppIfNecessary || restartAppIfNecessary(211820)) {
    Logger::error("Failed check with Steam.");
    return false;
  }
  return true;
}

}
