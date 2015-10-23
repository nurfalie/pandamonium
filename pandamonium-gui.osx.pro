CONFIG += qt release thread warn_on
LANGUAGE = C++
QT += network sql widgets
TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4) {
QT += concurrent
}

QMAKE_CLEAN += pandamonium
QMAKE_CXXFLAGS_RELEASE += -Wall -Wcast-align -Wcast-qual -Werror \
                          -Wextra -Wpointer-arith \
                          -Wstack-protector -Wstrict-overflow=5 \
                          -fPIE -fstack-protector-all -fwrapv

INCLUDEPATH +=
LIBS += -framework AppKit -framework Cocoa

FORMS = UI/pandamonium.ui \
	UI/pandamonium_broken_links.ui \
	UI/pandamonium_export_definition.ui \
        UI/pandamonium_statistics.ui \
	UI/pandamonium_statusbar.ui

HEADERS = Source/pandamonium-common.h \
	  Source/pandamonium-database.h \
	  Source/pandamonium-gui.h
SOURCES = Source/pandamonium-database.cc \
	  Source/pandamonium-gui.cc \
          Source/pandamonium-gui-main.cc

OBJECTIVE_HEADERS += Source/CocoaInitializer.h
OBJECTIVE_SOURCES += Source/CocoaInitializer.mm

ICON = Icons/pandamonium.icns
RESOURCES = Icons/icons.qrc \
	    Images/images.qrc
PROJECTNAME = pandamonium
TARGET = pandamonium

pandamonium.path        = /Applications/pandamonium.d/pandamonium.app
pandamonium.files       = pandamonium.app/*
macdeployqt.path	= pandamonium.app
macdeployqt.extra	= $$[QT_INSTALL_BINS]/macdeployqt ./pandamonium.app -verbose=0 2>/dev/null; echo;
preinstall.path         = /Applications/pandamonium.d
preinstall.extra        = rm -rf /Applications/pandamonium.d/pandamonium.app/*
postinstall.path	= /Applications/pandamonium.d
postinstall.extra	= cp -r pandamonium.app /Applications/pandamonium.d/.

INSTALLS	= preinstall \
		  macdeployqt \
		  pandamonium \
		  postinstall
