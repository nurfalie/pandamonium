purge.commands = rm -f *~ && rm -f */*~

CONFIG += ordered
QMAKE_EXTRA_TARGETS = purge
SUBDIRS = pandamonium-gui.pro \
          pandamonium-kernel.pro
TEMPLATE = subdirs
