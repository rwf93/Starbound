#!/bin/sh -e

cd "`dirname \"$0\"`/../../dist"

STEAM_INSTALL_DIR="$HOME/Library/Application Support/Steam/steamapps/common/Starbound - Unstable"

if [[ ! -d "$STEAM_INSTALL_DIR" ]]; then
  echo "Steam install directory not found: $STEAM_INSTALL_DIR"
  exit 1
fi

echo "Copying files to Steam install directory: $STEAM_INSTALL_DIR"
cp ../assets/packed.pak "$STEAM_INSTALL_DIR/assets/packed.pak"
cp libsteam_api.dylib "$STEAM_INSTALL_DIR/osx/libsteam_api.dylib"
cp libdiscord_game_sdk.dylib "$STEAM_INSTALL_DIR/osx/libdiscord_game_sdk.dylib"
cp starbound "$STEAM_INSTALL_DIR/osx/Starbound.app/Contents/MacOS/"
