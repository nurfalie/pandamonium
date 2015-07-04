Please use Qt 5.x and newer. Qt 4.8.7 is supported, however, QUrl::toEncoded()
may not properly escape quotes!

Non-OS X

qmake -o Makefile pandemonium.pro
make (gmake, mingw32-make)

OS X

qmake -o Makefile pandemonium.osx.pro
make
