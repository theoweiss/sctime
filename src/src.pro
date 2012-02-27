TEMPLATE = app
CONFIG += warn_on qt uic
# you have to explicitly recompile sctime.cpp whenever you change this value
VERSION = 0.73.6
DEFINES += APP_VERSION=$$VERSION
QT += xml gui core network sql
TARGET = sctime
CODECFORTR= UTF-8
TRANSLATIONS = sctime_de.ts
SOURCES = abteilungsliste.cpp bereitschaftsliste.cpp bereitschaftsmodel.cpp bereitschaftsview.cpp datasource.cpp datedialog.cpp defaultcommentreader.cpp defaulttagreader.cpp descdata.cpp findkontodialog.cpp GetOpt.cpp kontotreeitem.cpp kontotreeview.cpp lock.cpp preferencedialog.cpp qdatepicker.cpp sctime.cpp sctimexmlsettings.cpp setupdsm.cpp timemainwindow.cpp unterkontodialog.cpp
HEADERS = abteilungsliste.h bereitschaftsliste.h bereitschaftsmodel.h bereitschaftsview.h datasource.h datedialog.h defaultcommentreader.h defaulttagreader.h descdata.h eintragsliste.h findkontodialog.h GetOpt.h globals.h kontodateninfo.h kontoliste.h kontotreeitem.h kontotreeview.h lock.h preferencedialog.h qcalendarsystem.h qdatepicker.h sctimexmlsettings.h setupdsm.h statusbar.h timecounter.h timeedit.h timemainwindow.h unterkontodialog.h unterkontoeintrag.h unterkontoliste.h
RESOURCES= ../pics/sctimeImages.qrc translations.qrc
FORMS = datedialogbase.ui preferencedialogbase.ui
INSTALLS += target
win32{
  CONFIG += embed_manifest_exe
  #QMAKE_CXXFLAGS += -EHsc # C++-Ausnahmen
  RC_FILE += sctime.rc
  DEFINES += _CRT_SECURE_NO_WARNINGS
  }
!win32{
  SOURCES += unix/signalhandler.cpp
  HEADERS += unix/signalhandler.h
}
hpux-acc{
  LIBS += -L/opt/graphics/OpenGL/lib $$QMAKE_LIBS_OPENGL
}
solaris-cc{
  QMAKE_CXXFLAGS_RELEASE += -features=conststrings
  LIBS += -ldl $$QMAKE_LIBS_OPENGL
}
solaris-g++{
  LIBS += -ldl
}
irix-cc{
  QMAKE_LIBS_QT += -lGL
}
mac {
  # with Qt-4.8.0 icon and Info.plist rules do not work with out-of-source
  # builds. try again later... sctime-mac-dist does this for now
  #ICON = $$PWD/../pics/scTime.icns
  #QMAKE_INFO_PLIST = $$PWD/../extra/mac/Info.plist
  TARGET = scTime
  QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
  DEFINES += MACOS
  CONFIG += x86_64 x86 ppc
}
