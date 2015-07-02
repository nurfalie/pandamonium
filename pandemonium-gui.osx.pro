CONFIG += qt release thread warn_on
LANGUAGE = C++
QT += network sql widgets
TEMPLATE = app

QMAKE_CLEAN += pandemonium
QMAKE_CXXFLAGS_RELEASE += -Wall -Wcast-align -Wcast-qual -Werror \
                          -Wextra -Wpointer-arith \
                          -Wstack-protector -Wstrict-overflow=5 \
                          -fPIE -fstack-protector-all -fwrapv

INCLUDEPATH +=
LIBS += -framework AppKit -framework Cocoa

FORMS = UI/pandemonium.ui \
	UI/pandemonium_export_definition.ui \
        UI/pandemonium_statistics.ui        

HEADERS = Source/pandemonium-common.h \
	  Source/pandemonium-database.h \
	  Source/pandemonium-gui.h
SOURCES = Source/pandemonium-database.cc \
	  Source/pandemonium-gui.cc \
          Source/pandemonium-gui-main.cc

OBJECTIVE_HEADERS += Source/CocoaInitializer.h
OBJECTIVE_SOURCES += Source/CocoaInitializer.mm

ICON = Icons/pandemonium.icns
RESOURCES = Icons/icons.qrc
PROJECTNAME = pandemonium
TARGET = pandemonium

pandemonium.path        = /Applications/pandemonium.d/pandemonium.app
pandemonium.files       = pandemonium.app/*
macdeployqt.path	= pandemonium.app
macdeployqt.extra	= $$[QT_INSTALL_BINS]/macdeployqt ./pandemonium.app -verbose=0 2>/dev/null; echo;
preinstall.path         = /Applications/pandemonium.d
preinstall.extra        = rm -rf /Applications/pandemonium.d/pandemonium.app/*
postinstall.path	= /Applications/pandemonium.d
postinstall.extra	= cp -r pandemonium.app /Applications/pandemonium.d/.

INSTALLS	= preinstall \
		  macdeployqt \
		  pandemonium \
		  postinstall
