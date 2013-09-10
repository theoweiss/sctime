SUBDIRS += src
TEMPLATE = subdirs
CONFIG += release warn_on qt

cache()

mac {
  SUBDIRS += extra/mac
  extra-mac.depends = src
}
