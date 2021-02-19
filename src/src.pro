TEMPLATE = app
CONFIG += warn_on qt uic
CONFIG += c++11
# you have to explicitly recompile sctime.cpp whenever you change this value
VERSION = \\\"$$system(git describe --always||echo UNDEFINED)\\\"
DEFINES += APP_VERSION=$$VERSION
QT += xml gui core network sql widgets
TARGET = sctime
CODECFORTR= UTF-8
TRANSLATIONS = sctime_de.ts
SOURCES = abteilungsliste.cpp bereitschaftsliste.cpp bereitschaftsmodel.cpp bereitschaftsview.cpp\
          datasource.cpp datedialog.cpp defaultcomment.cpp defaultcommentreader.cpp defaulttagreader.cpp\
          descdata.cpp findkontodialog.cpp kontotreeitem.cpp kontotreeview.cpp lock.cpp preferencedialog.cpp\
          sctime.cpp sctimexmlsettings.cpp setupdsm.cpp timemainwindow.cpp unterkontodialog.cpp\
          specialremunerationsdialog.cpp specialremuntypemap.cpp statusbar.cpp JSONReader.cpp\
          textviewerdialog.cpp sctimeapp.cpp
HEADERS = abteilungsliste.h bereitschaftsliste.h bereitschaftsmodel.h bereitschaftsview.h datasource.h\
          datedialog.h defaultcomment.h defaultcommentreader.h defaulttagreader.h descdata.h eintragsliste.h\
          findkontodialog.h globals.h kontodateninfo.h kontoliste.h kontotreeitem.h kontotreeview.h lock.h\
          preferencedialog.h sctimexmlsettings.h setupdsm.h statusbar.h timecounter.h timeedit.h timemainwindow.h\
          unterkontodialog.h unterkontoeintrag.h unterkontoliste.h specialremunerationsdialog.h\
          specialremuntypemap.h JSONReader.h util.h textviewerdialog.h sctimeapp.h
RESOURCES = ../pics/sctimeImages.qrc ../help/help.qrc
GENERATED_RESOURCES = translations.qrc
FORMS = datedialogbase.ui preferencedialogbase.ui specialremunerationdialogbase.ui

# just tell qmake that qrc_generated_translations.cpp depends on all
# translations qm files - yes, this is somewhat bulky
qrc_generated_translations_cpp.target = qrc_generated_translations.cpp
qrc_generated_translations_cpp.depends = $$replace(TRANSLATIONS, ".ts", ".qm")
QMAKE_EXTRA_TARGETS += qrc_generated_translations_cpp

# add a custom compiler that copies resource files that depend on generated
# files into the build dir so that the generated files can be included from
# there.
copytobuilddir.input = GENERATED_RESOURCES
copytobuilddir.output = generated_${QMAKE_FILE_BASE}.qrc
copytobuilddir.commands = $$QMAKE_COPY "${QMAKE_FILE_IN}" "${QMAKE_FILE_OUT}"
copytobuilddir.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += copytobuilddir

# add a custom resource compiler that expects the resource files inside the
# build dir. This way they can include files generated by the build which is
# used for translation files that are lreleased automatically as part of the
# build process.
defined(qtPrepareTool)|load(qt_functions)
defined(qtPrepareTool)|defineTest(qtPrepareTool) {
	isEmpty($$1) {
		!isEmpty(QT_BUILD_TREE):$$1 = $$QT_BUILD_TREE/bin/$$2
		else:$$1 = $$[QT_INSTALL_BINS]/$$2
	}
	$$1 ~= s,[/\\\\],$$QMAKE_DIR_SEP,
	contains(QMAKE_HOST.os, Windows):!contains($$1, .*\\.(exe|bat)$) {
		exists($$eval($$1).bat) {
		$$1 = $$eval($$1).bat
	} else {
		$$1 = $$eval($$1).exe
		}
	}
	export($$1)
}
qtPrepareTool(QMAKE_RCC, rcc)
for(resource, GENERATED_RESOURCES) {
	# file names in build dir *have* to differ from file names in source
	# dir. Otherwise generatedrcc.input below will find the source dir
	# files and put them into the make rules instead of the generated files
	# in the build dir. So just prepent generated_ to make file names
	# differ. Unfortunately this also causes a warning for each file:
	# WARNING: Failure to find: generated_translations.qrc
	GENERATED_RESOURCES_IN_BUILD_DIR += generated_$$basename(resource)
}
generatedrcc.input = GENERATED_RESOURCES_IN_BUILD_DIR
generatedrcc.output = qrc_${QMAKE_FILE_BASE}.cpp
generatedrcc.commands = "$$QMAKE_RCC" -name "${QMAKE_FILE_IN_BASE}" "${QMAKE_FILE_IN_BASE}.qrc" -o "${QMAKE_FILE_OUT}"
generatedrcc.variable_out = GENERATED_SOURCES
QMAKE_EXTRA_COMPILERS += generatedrcc
# Visual-Studio-specific workaround #1: The qmake vcproj template does not
# support the QMAKE_EXTRA_TARGETS mechanism we use above to define the
# dependency of qrc_generated_translations.cpp upon sctime_de.qm. We work
# around that by just making every qrc_*.cpp depend on every *.qm. This
# obviously is broken from a dependency point of view but serves our purpose
# well enough.
contains(TEMPLATE, vcapp):generatedrcc.depends = $$replace(TRANSLATIONS, ".ts", ".qm")

# add a custom compiler that creates .qm files from .ts translation files
qtPrepareTool(QMAKE_LRELEASE, lrelease)
lrelease.input = TRANSLATIONS
lrelease.output = ${QMAKE_FILE_BASE}.qm
lrelease.commands = "$$QMAKE_LRELEASE" "${QMAKE_FILE_IN}" -qm "${QMAKE_FILE_OUT}"
# Visual-Studio-specific workaround #2: Not adding the output file name to some
# variable will cause the vcproj template to add the extra compiler rules to
# the project *twice*: Once in the Translation Files filter (based on the
# TRANSLATIONS variable) and once in our custom compiler's lrelease filter.
# This seems to confuse Visual Studio's dependency resolution into not building
# sctime_*.qm files at all. By adding the output file name to some dummy
# variable, the lrelease filter and second set of rules is suppressed and
# building works as desired via the Translation Files filter's rules.
lrelease.variable_out = RELEASED_TRANSLATIONS
lrelease.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += lrelease

win32-msvc*{
  CONFIG += embed_manifest_exe
  RC_FILE += sctime.rc
  #QMAKE_CXXFLAGS += -EHsc # C++-Ausnahmen
  DEFINES += _CRT_SECURE_NO_WARNINGS
  # When using nmake, we need to add advapi32.lib (Visual Studio automatically
  # adds a huge list of libs to the linker call). Otherwise we get:
  # setupdsm.obj : error LNK2019: unresolved external symbol
  # __imp__GetUserNameA@8 referenced in function "class QString __cdecl
  # username(void)" (?username@@YA?AVQString@@XZ)
  LIBS += advapi32.lib
}
win32-g++*{
  CONFIG += embed_manifest_exe
  RC_FILE += sctime.rc
  LIBS += -ladvapi32
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
  # with Qt-5.0 icon and Info.plist rules still do not work with out-of-source
  # builds. try again later... sctime-mac-dist does this for now
  #ICON = $$PWD/../pics/scTime.icns
  #QMAKE_INFO_PLIST = $$PWD/../extra/mac/Info.plist
  TARGET = scTime
  QMAKE_MAC_SDK = macosx10.12
}
