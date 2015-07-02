CONFIG += qt release warn_on
LANGUAGE = C++
QT += network sql widgets
TEMPLATE = app

QMAKE_CLEAN += pandemonium-kernel

macx {
QMAKE_CXXFLAGS_RELEASE += -Wall -Wcast-align -Wcast-qual \
                          -Werror -Wextra \
                          -Woverloaded-virtual -Wpointer-arith \
                          -Wstack-protector -Wstrict-overflow=5 \
                          -fPIE -fstack-protector-all -fwrapv
} else
win32 {
QMAKE_CXXFLAGS_RELEASE += -Wall -Wcast-align -Wcast-qual -Werror \
                          -Wextra -Wpointer-arith \
                          -Wstack-protector -Wstrict-overflow=5 \
                          -fwrapv -pie
} else
!macx {
QMAKE_CXXFLAGS_RELEASE += -Wall -Wcast-align -Wcast-qual \
                          -Werror -Wextra -Wl,-z,relro \
                          -Woverloaded-virtual -Wpointer-arith \
                          -Wstack-protector -Wstrict-overflow=5 \
                          -fPIE -fstack-protector-all -fwrapv -pie
}

INCLUDEPATH += . Source

macx {
LIBS += -framework Cocoa
}

HEADERS = Source\\pandemonium-common.h \
	  Source\\pandemonium-database.h \
          Source\\pandemonium-kernel.h \
          Source\\pandemonium-kernel-url.h

SOURCES = Source\\pandemonium-database.cc \
          Source\\pandemonium-kernel.cc \
	  Source\\pandemonium-kernel-main.cc \
          Source\\pandemonium-kernel-url.cc

macx {
OBJECTIVE_HEADERS += Source/CocoaInitializer.h
OBJECTIVE_SOURCES += Source/CocoaInitializer.mm
}

PROJECTNAME = pandemonium-kernel
TARGET = pandemonium-kernel

macx {
pandemonium-kernel.path        = /Applications/pandemonium.d/pandemonium-kernel.app
pandemonium-kernel.files       = pandemonium-kernel.app/*
macdeployqt.path	= pandemonium-kernel.app
macdeployqt.extra	= $$[QT_INSTALL_BINS]/macdeployqt ./pandemonium-kernel.app -verbose=0 2>/dev/null; echo;
preinstall.path         = /Applications/pandemonium.d
preinstall.extra        = rm -rf /Applications/pandemonium.d/pandemonium-kernel.app/*
postinstall.path	= /Applications/pandemonium.d
postinstall.extra	= cp -r pandemonium-kernel.app /Applications/pandemonium.d/.

INSTALLS	= preinstall \
		  macdeployqt \
		  pandemonium-kernel \
                  postinstall
}
