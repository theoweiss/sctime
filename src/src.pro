TEMPLATE = app
VERSIONSTR = \"0.41\"
CONFIG  += release warn_on qt
OBJECTS_DIR = ../obj
TARGET = ../bin/sctime
target.path = $(prefix)/bin
INSTALLS += target

SOURCES += abteilungsliste.cpp datedialog.cpp \
           unterkontodialog.cpp kontotreeview.cpp findkontodialog.cpp \
           timemainwindow.cpp sctimexmlsettings.cpp sctime.cpp

HEADERS += abteilungsliste.h kontoliste.h statusbar.h datedialog.h \
           kontotreeitem.h timecounter.h eintragsliste.h \
           kontotreetooltip.h timeedit.h errorapp.h kontotreeview.h \
           time.h findkontodialog.h timemainwindow.h globals.h \
           toolbar.h kontodateninfodatabase.h sctimeapp.h \
           unterkontodialog.h kontodateninfo.h sctimehelp.h unterkontoeintrag.h \
           kontodateninfozeit.h sctimexmlsettings.h unterkontoliste.h

IMAGES += ../pics/hi16_action_apply.xpm \
          ../pics/hi22_action_1downarrow.xpm \
          ../pics/hi22_action_1uparrow.xpm \
          ../pics/hi22_action_2downarrow.xpm \
          ../pics/hi22_action_2uparrow.xpm \
          ../pics/hi22_action_attach.xpm \
          ../pics/hi22_action_edit.xpm \
          ../pics/hi22_action_filesave.xpm \
          ../pics/hi22_action_player_pause_half.xpm \
          ../pics/hi22_action_player_pause.xpm \
          ../pics/hi22_action_queue.xpm \
          ../pics/scLogo_15Farben.xpm \
          ../pics/sc_logo.xpm

linux-g++-static {
  LIBS += -ldl
}

linux-g++ {
  LIBS += -ldl
}

!win32 {
    BUILDDATESTR = \""`date`"\"
    SOURCES += kontodateninfozeit.cpp
}

win32 {
  DEFINES += WIN32
  QMAKE_CXXFLAGS += -GX
  SOURCES += kontodateninfodatabase.cpp
}

hpux-acc {
  LIBS += -L/opt/graphics/OpenGL/lib $$QMAKE_LIBS_OPENGL
}

solaris-cc {
  DEFINES += SUN
  QMAKE_CXXFLAGS_RELEASE += -features=conststrings
  LIBS += -ldl $$QMAKE_LIBS_OPENGL
}

irix-n32 {
  DEFINES += IRIX
  QMAKE_CXXFLAGS += -LANG:std
  QMAKE_LFLAGS += -LANG:std
  # der Compiler benoetigt -lGL nach -lqt
  QMAKE_LIBS_QT += -lGL
}

isEmpty(BUILDDATESTR): BUILDDATESTR = \""unknown"\"
DEFINES += BUILDDATESTR=$$BUILDDATESTR VERSIONSTR=$$VERSIONSTR
