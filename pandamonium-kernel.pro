CONFIG += qt release warn_on
LANGUAGE = C++
QT += network sql widgets
TEMPLATE = app

QMAKE_CLEAN += pandamonium-kernel

win32 {
QMAKE_CXXFLAGS_RELEASE += -Wall -Wcast-align -Wcast-qual -Werror \
                          -Wextra -Wpointer-arith \
                          -Wstack-protector -Wstrict-overflow=5 \
                          -fwrapv -pie
}
else {
QMAKE_CXXFLAGS_RELEASE += -Wall -Wcast-align -Wcast-qual \
                          -Werror -Wextra -Wl,-z,relro \
                          -Woverloaded-virtual -Wpointer-arith \
                          -Wstack-protector -Wstrict-overflow=5 \
                          -fPIE -fstack-protector-all -fwrapv -pie
}

INCLUDEPATH += . Source

HEADERS = Source\\pandamonium-common.h \
	  Source\\pandamonium-database.h \
          Source\\pandamonium-kernel.h \
          Source\\pandamonium-kernel-url.h

SOURCES = Source\\pandamonium-database.cc \
          Source\\pandamonium-kernel.cc \
	  Source\\pandamonium-kernel-main.cc \
          Source\\pandamonium-kernel-url.cc

PROJECTNAME = pandamonium-kernel
TARGET = pandamonium-kernel
