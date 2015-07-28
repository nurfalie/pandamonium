CONFIG += qt release thread warn_on
LANGUAGE = C++
QT += network sql widgets
TEMPLATE = app

QMAKE_CLEAN += pandamonium

win32 {
QMAKE_CXXFLAGS_RELEASE += -Wall -Wcast-align -Wcast-qual -Werror \
                          -Wextra -Wpointer-arith \
                          -Wstack-protector -Wstrict-overflow=5 \
                          -fwrapv -pie
}
else {
QMAKE_CXXFLAGS_RELEASE += -Wall -Wcast-align -Wcast-qual -Werror \
                          -Wextra -Wpointer-arith \
                          -Wstack-protector -Wstrict-overflow=5 \
                          -fPIE -fstack-protector-all -fwrapv -pie
}

INCLUDEPATH +=

FORMS = UI\\pandamonium.ui \
	UI\\pandamonium_export_definition.ui \
        UI\\pandamonium_statistics.ui \
	UI\\pandamonium_statusbar.ui

HEADERS = Source\\pandamonium-common.h \
	  Source\\pandamonium-database.h \
	  Source\\pandamonium-gui.h
SOURCES = Source\\pandamonium-database.cc \
	  Source\\pandamonium-gui.cc \
          Source\\pandamonium-gui-main.cc

win32 {
RC_FILE = pandamonium.rc
}

RESOURCES = Icons\\icons.qrc \
	    Images\\images.qrc

PROJECTNAME = pandamonium
TARGET = pandamonium
