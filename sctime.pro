SUBDIRS += src
TEMPLATE = subdirs
CONFIG += release warn_on qt
CONFIG += c++11

cache()

mac {
  SUBDIRS += extra/mac
  extra-mac.depends = src
}
