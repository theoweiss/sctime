sctime
Version 0.61
01.10.2010

sctime provides a graphical interface which allows users to record the times used for
specific projects.

DEVELOPER:
Florian Schmitt f.schmitt@science-computing.de

REPOSITORY:


REQUIREMENTS (for running sctime):
Qt 4.4 or newer
On UNIX, you need "zeitkonten" and "zeitbereitls".
Un Windows, you need "qsqlodbc4.dll" and/or "qsqlpsql4.dll" (QT plugins/sqldrivers).

BUILD:

Unix
----
create a build directory, go there, then:
$ qmake -r .../sctime.pro

Windows
-------
Requirements: Microsoft Visual Studio 2008 or newer
create a build directory, go there, setup the Qt environment, then:
$ qmake -tp vc -r ...\sctime.pro
$ sctime.sln
choose "Release" build!


INSTALL:
All you need is the executable sctime, which you may copy
to a directory in your PATH variable.

TESTING:
Check the *.sh scripts!

DOCUMENTATION:
The source is documented in Docbook style. You might also want to check
the menu entry "Hilfe".

MAILING LISTS:
sctime-user@lists.sourceforge.net

CUSTOMER:
s+c and the public.

COPYRIGHT:
GNU Public License, see GPL



Windows
=======

Notwendige Dateien
------------------

Die notwendigen Dateien/Konfigurationen für den Datenbankzugriff im Überblick:

- QtSql4.dll
- sqldrivers\qsqlodbc4.dll und/oder sqldrivers\qsqlpsql4.dll

Aus dem "plugins"-Verzeichnis der kommerziellen Qt-Distribution brraucht man
das "sqldrivers"-Verzeichnis mit den Dateien "qsqlodbc4.dll" (für ODBC) oder
"qsqlpsql4.dll" (für den direkten Zugriff auf Postgres-Datenbanken).
Das Programm probiert den Verbindungsaufbau erst mit der ODBC-DSN
"Postgres_Zeit, wofür der Postgres-ODBC-Treiber installiert sein muss, und die
DSN konfiguriert sein muss. Wenn das nicht klappt, probiert's Sctime mit einer
Postgres-Direktverbindung.

Ein Nachteil dieses Verfahrens ist, dass man bei Problemen nur die
Fehlermeldungen des zweiten Verbindungsaufbaus sieht.

Weiter notwendige Bibliotheken:

- QtGui4.dll
- QtXml4.dll
- QtCore4.dll
- QtSql4.dll (siehe oben)
- QtNetwork4.dll

- Microsoft Visual C++ 2008 SDK


"shadow" / "out of source"-Build mit Visual Studio 2008
-------------------------------------------------------

Ein eigenes Verzeichnis anlegen, dann dort in einer Shell mit
Qt-Umgebungsvariablen:

$ qmake -tp vc -r QUELLVERZEICHNIS\sctime.pro

Dann die "solution" öffnen:
- sctime.sln
oder (gleich?)
- Visual Studio in dieser Umgebung öffnen

Danach unter "Build"/"Configuration Manager" auf "Release" umstellen. Für
die Konfiguration "Debug" braucht man natürlich die Debug-Bibliothken.

Debuggen war nicht möglich: "Access violation"


MSI-Paket bauen
----------------

http://appdeveloper.intel.com/en-us/article/how-creating-your-msi-installer-using-visual-studio-2008
