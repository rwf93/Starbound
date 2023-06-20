#!/bin/sh

sudo apt-get install libssl-dev libicu-dev bison gperf build-essential

cd $HOME/Qt/5.3/Src
./configure -openssl-linked -qt-pcre -qt-zlib -qt-libpng -no-mtdev -no-iconv -no-cups -fontconfig -no-nis -no-evdev -no-eglfs -dbus-linked -no-separate-debug-info -qt-xcb -opensource -confirm-license -no-xrandr -no-xinerama -no-xcursor -no-xfixes -qt-libjpeg -qt-freetype -prefix $PWD/qtbase -nomake examples -nomake tests
make install
