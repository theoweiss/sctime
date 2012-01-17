SUBDIRS += src
TEMPLATE = subdirs
CONFIG += release warn_on qt
#OBJECTS_DIR = ./obj

mac {
  SUBDIRS += extra/mac
  extra-mac.depends = src
}
