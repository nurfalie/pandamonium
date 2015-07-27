Please see https://github.com/textbrowser/pandamonium/wiki for usage hints.

Please use Qt 5.x or newer. Qt 4.8.x is supported, however, QUrl::toEncoded()
may not properly escape quotes!

Non-OS X

qmake -o Makefile pandamonium.pro
make (gmake, mingw32-make)

OS X

qmake -o Makefile pandamonium.osx.pro
make
