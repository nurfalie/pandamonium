CONFIG += qt release thread warn_on
LANGUAGE = C++
QT += network sql widgets
TEMPLATE = app

QMAKE_CLEAN += pandemonium

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

FORMS = UI\\pandemonium.ui \
        UI\\pandemonium_statistics.ui

HEADERS = Source\\pandemonium-common.h \
	  Source\\pandemonium-database.h \
	  Source\\pandemonium-gui.h
SOURCES = Source\\pandemonium-database.cc \
	  Source\\pandemonium-gui.cc \
          Source\\pandemonium-gui-main.cc

RESOURCES = Icons/icons.qrc
PROJECTNAME = pandemonium
TARGET = pandemonium
