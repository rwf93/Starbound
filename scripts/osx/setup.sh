#!/bin/sh

cd "`dirname \"$0\"`/../.."

mkdir -p dist
cp scripts/osx/sbinit.config dist/

mkdir -p build
cd build

if [ -d build ]; then
  echo "-- Cleaning up previous build --"
  rm -rf build
fi

echo "-- Checking if brew is installed --"
if ! command -v brew &> /dev/null; then
  echo "-- Brew is not installed --"
  echo "-- Installing brew --"
  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

echo "-- Updating brew --"
brew update

echo "-- Checking if dependencies are installed --"
if ! brew list cmake &> /dev/null; then
  echo "-- Installing cmake --"
  brew install cmake
elif ! brew list ninja &> /dev/null; then
  echo "-- Installing ninja --"
  brew install ninja
elif ! brew list jemalloc &> /dev/null; then
  echo "-- Installing jemalloc --"
  brew install jemalloc
elif ! brew list sdl2 &> /dev/null; then
  echo "-- Installing sdl2 --"
  brew install sdl2
elif ! brew list glew &> /dev/null; then
  echo "-- Installing glew --"
  brew install glew
elif ! brew list libvorbis &> /dev/null; then
  echo "-- Installing libvorbis --"
  brew install libvorbis
elif ! brew list lzlib &> /dev/null; then
  echo "-- Installing lzlib --"
  brew install lzlib
elif ! brew list libpng &> /dev/null; then
  echo "-- Installing libpng --"
  brew install libpng
elif ! brew list freetype &> /dev/null; then
  echo "-- Installing freetype --"
  brew install freetype
fi
echo "-- Checking dependencies done --"

QT5_INSTALL_PATH=/usr/local/opt/qt5
if [ -d $QT5_INSTALL_PATH ]; then
  export PATH=$QT5_INSTALL_PATH/bin:$PATH
  export LDFLAGS=-L$QT5_INSTALL_PATH/lib
  export CPPFLAGS=-I$QT5_INSTALL_PATH/include
  export CMAKE_PREFIX_PATH=$QT5_INSTALL_PATH
  BUILD_QT_TOOLS=ON
else
  BUILD_QT_TOOLS=OFF
fi

echo "-- Choose your architecture --"
echo "1. x86_64 (universal macs (intel, M1, M2, M3)"
echo "2. arm64 (sillicon macs (M1, M2, M3))"
read -p "Enter your choice: " choice
case $choice in
  1)
    export CMAKE_OSX_ARCHITECTURES=x86_64
    export CMAKE_ARCH_INCLUDE=../lib/osx/include
    export CMAKE_ARCH_LIB=../lib/osx
    ;;
  2)
    export CMAKE_OSX_ARCHITECTURES=arm64
    export CMAKE_ARCH_INCLUDE=../lib/osx/arm64/include
    export CMAKE_ARCH_LIB=../lib/osx/arm64
    ;;
  *)
    echo "-- Invalid choice --"
    exit 1
    ;;
esac  

echo "-- Building Starbound --"

CC=clang CXX=clang++ cmake \
  -DCMAKE_OSX_ARCHITECTURES=$CMAKE_OSX_ARCHITECTURES \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=true \
  -DCMAKE_BUILD_TYPE=RelWithAsserts \
  -DSTAR_BUILD_QT_TOOLS=$BUILD_QT_TOOLS \
  -DSTAR_USE_JEMALLOC=ON \
  -DSTAR_ENABLE_STEAM_INTEGRATION=ON \
  -DSTAR_ENABLE_DISCORD_INTEGRATION=ON \
  -DCMAKE_INCLUDE_PATH=$CMAKE_ARCH_INCLUDE \
  -DCMAKE_LIBRARY_PATH=$CMAKE_ARCH_LIB \
  ../source

mkdir -p ../dist
cd ../dist

echo "-- Copying resources --"

cp ../scripts/steam_appid.txt .

echo "-- Copying libraries --"

cp $CMAKE_ARCH_LIB/libsteam_api.dylib .
cp $CMAKE_ARCH_LIB/libdiscord_game_sdk.dylib .

echo "-- Building app --"
make -j$(sysctl -n hw.logicalcpu) -C../build || exit 1

echo "-- Done --"