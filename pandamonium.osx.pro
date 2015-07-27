purge.commands = rm -f *~ && rm -f */*~

CONFIG += ordered
QMAKE_EXTRA_TARGETS = purge
SUBDIRS = pandamonium-gui.osx.pro \
          pandamonium-kernel.osx.pro
TEMPLATE = subdirs
