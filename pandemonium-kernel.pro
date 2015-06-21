CONFIG += qt release warn_on
LANGUAGE = C++
QT += network sql webkit
TEMPLATE = app

QMAKE_CLEAN += pandemonium-kernel

win32 {
QMAKE_CXXFLAGS_RELEASE += -Wall -Wcast-align -Wcast-qual -Werror \
                          -Wextra -Wpointer-arith \
                          -Wstack-protector -Wstrict-overflow=5 \
                          -fstack-protector-all -fwrapv -pie
}
else {
QMAKE_CXXFLAGS_RELEASE += -Wall -Wcast-align -Wcast-qual \
                          -Werror -Wextra -Wl,-z,relro \
                          -Woverloaded-virtual -Wpointer-arith \
                          -Wstack-protector -Wstrict-overflow=5 \
                          -fPIE -fstack-protector-all -fwrapv -pie
}

INCLUDEPATH +=

HEADERS = Source/pandemonium-common.h \
	  Source/pandemonium-database.h \
          Source/pandemonium-kernel.h
SOURCES = Source/pandemonium-database.cc \
          Source/pandemonium-kernel.cc \
	  Source/pandemonium-kernel-main.cc

PROJECTNAME = pandemonium-kernel
TARGET = pandemonium-kernel
