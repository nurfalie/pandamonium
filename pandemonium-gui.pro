purge.commands = rm -f *~ && rm -f */*~

CONFIG += qt release thread warn_on
LANGUAGE = C++
QT += sql widgets
TEMPLATE = app

QMAKE_CLEAN += pandemonium
QMAKE_CXXFLAGS_RELEASE += -Wall -Werror -Wextra -Wpointer-arith \
                          -Wstack-protector -Wstrict-overflow=5 \
                          -fPIE -fstack-protector-all -fwrapv -pie
QMAKE_EXTRA_TARGETS = purge

INCLUDEPATH +=

FORMS = UI/pandemonium.ui

HEADERS = Source/pandemonium.h \
	  Source/pandemonium_database.h
SOURCES = Source/pandemonium.cc \
          Source/pandemonium_database.cc \
          Source/pandemonium_main.cc \

PROJECTNAME = pandemonium
TARGET = pandemonium
