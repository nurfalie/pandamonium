Please also see https://github.com/textbrowser/pandemonium/wiki.

Please use Qt 5.x or newer. Qt 4.8.x is supported, however, QUrl::toEncoded()
may not properly escape quotes!

Non-OS X

qmake -o Makefile pandemonium.pro
make (gmake, mingw32-make)

OS X

qmake -o Makefile pandemonium.osx.pro
make
