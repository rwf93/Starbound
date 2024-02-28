#!/bin/sh

cd "`dirname \"$0\"`/../.."

mkdir -p dist
cp scripts/osx/sbinit.config dist/

mkdir -p build
cd build

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

arch=$(uname -m)
if [[ "$arch" == "arm64" ]]; then
  LIB_PATH_ARCH=../lib/osx/arm64
else
  LIB_PATH_ARCH=../lib/osx
fi

CC=gcc CXX=g++ cmake \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=true \
  -DCMAKE_BUILD_TYPE=RelWithAsserts \
  -DSTAR_BUILD_QT_TOOLS=$BUILD_QT_TOOLS \
  -DSTAR_USE_JEMALLOC=ON \
  -DSTAR_ENABLE_STEAM_INTEGRATION=OFF \
  -DSTAR_ENABLE_DISCORD_INTEGRATION=OFF \
  -DCMAKE_INCLUDE_PATH=$LIB_PATH_ARCH/include \
  -DCMAKE_LIBRARY_PATH=$LIB_PATH_ARCH \
  ../source
