SUBDIRS += src
TEMPLATE = subdirs
CONFIG += release warn_on qt

mac {
  SUBDIRS += extra/mac
  extra-mac.depends = src
}
