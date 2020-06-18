CONFIG += qt release warn_on
LANGUAGE = C++
QT += network sql widgets
TEMPLATE = app

QMAKE_CLEAN += pandamonium-kernel
QMAKE_CXXFLAGS_RELEASE += -Wall \
                          -Wcast-align \
                          -Wcast-qual \
                          -Werror \
                          -Wextra \
                          -Woverloaded-virtual \
                          -Wpointer-arith \
                          -Wstack-protector \
                          -Wstrict-overflow=5 \
                          -fPIE \
                          -fstack-protector-all \
                          -fwrapv

INCLUDEPATH += . Source
LIBS += -framework Cocoa
HEADERS = Source/pandamonium-common.h \
	  Source/pandamonium-database.h \
          Source/pandamonium-kernel.h \
          Source/pandamonium-kernel-url.h
SOURCES = Source/pandamonium-database.cc \
          Source/pandamonium-kernel.cc \
	  Source/pandamonium-kernel-main.cc \
          Source/pandamonium-kernel-url.cc

OBJECTIVE_HEADERS += Source/CocoaInitializer.h
OBJECTIVE_SOURCES += Source/CocoaInitializer.mm

PROJECTNAME = pandamonium-kernel
TARGET = pandamonium-kernel

macdeployqt.extra	= $$[QT_INSTALL_BINS]/macdeployqt ./pandamonium-kernel.app -verbose=0 2>/dev/null; echo;
macdeployqt.path	= pandamonium-kernel.app
pandamonium-kernel.files       = pandamonium-kernel.app/*
pandamonium-kernel.path        = /Applications/pandamonium.d/pandamonium-kernel.app
postinstall.extra	= cp -r pandamonium-kernel.app /Applications/pandamonium.d/.
postinstall.path	= /Applications/pandamonium.d
preinstall.extra        = rm -rf /Applications/pandamonium.d/pandamonium-kernel.app/*
preinstall.path         = /Applications/pandamonium.d

INSTALLS	= preinstall \
		  macdeployqt \
		  pandamonium-kernel \
		  postinstall
