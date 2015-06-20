CONFIG += qt release thread warn_on
LANGUAGE = C++
QT += sql webkit
TEMPLATE = app

QMAKE_CLEAN += pandemonium-kernel
QMAKE_CXXFLAGS_RELEASE += -Wall -Werror -Wextra -Wpointer-arith \
                          -Wstack-protector -Wstrict-overflow=5 \
                          -fPIE -fstack-protector-all -fwrapv -pie

INCLUDEPATH +=

HEADERS = Source/pandemonium-common.h \
	  Source/pandemonium-database.h \
          Source/pandemonium-kernel.h
SOURCES = Source/pandemonium-database.cc \
          Source/pandemonium-kernel.cc \
	  Source/pandemonium-kernel-main.cc

PROJECTNAME = pandemonium-kernel
TARGET = pandemonium-kernel
