sctime
======

sctime provides a graphical interface which allows users to record the time
spent for specific projects.

REQUIREMENTS (for running sctime):

Qt 4.4 or newer

Un Windows, you need "qsqlodbc4.dll" and/or "qsqlpsql4.dll" (QT plugins/sqldrivers).

On UNIX, you need "zeitkonten" and "zeitbereitls" or one of the plugins
mentioned for Windows.


BUILD:

out-of-source builds

Unix
----
create a build directory, go there, then:
$ mkdir bindir
$ qmake -r PROJDIR/sctime.pro


Windows
-------
Requirements: Microsoft Visual Studio 2008 or newer
create a build directory, go there, setup the Qt environment, then:
$ qmake -tp vc -r PROJDIR\sctime.pro
$ sctime.sln   # starts Visual Studio
choose "Release" build!


DOCUMENTATION:
User documentation is in the menu entry "Help".

The source is documented in Docbook style. Additional documentation for
developers is in directory "doc/". 

MAILING LISTS:
sctime-user@lists.sourceforge.net

CUSTOMER:
s+c and the public.

COPYRIGHT:
GNU Public License, see GPL


Windows-Build
-------------
see README.deutsch
