CONFIG += qt release warn_on
LANGUAGE = C++
QT += network sql widgets
TEMPLATE = app

QMAKE_CLEAN += pandemonium-kernel

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

HEADERS = Source\\pandemonium-common.h \
	  Source\\pandemonium-database.h \
          Source\\pandemonium-kernel.h \
          Source\\pandemonium-kernel-url.h

SOURCES = Source\\pandemonium-database.cc \
          Source\\pandemonium-kernel.cc \
	  Source\\pandemonium-kernel-main.cc \
          Source\\pandemonium-kernel-url.cc

PROJECTNAME = pandemonium-kernel
TARGET = pandemonium-kernel
