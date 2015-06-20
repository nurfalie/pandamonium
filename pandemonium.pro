purge.commands = rm -f *~ && rm -f */*~

CONFIG += ordered
QMAKE_EXTRA_TARGETS = purge
SUBDIRS = pandemonium-gui.pro \
          pandemonium-kernel.pro
TEMPLATE = subdirs
