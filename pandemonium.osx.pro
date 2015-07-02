purge.commands = rm -f *~ && rm -f */*~

CONFIG += ordered
QMAKE_EXTRA_TARGETS = purge
SUBDIRS = pandemonium-gui.osx.pro \
          pandemonium-kernel.osx.pro
TEMPLATE = subdirs
