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

Weitere notwendige Bibliotheken:

- QtGui4.dll
- QtXml4.dll
- QtCore4.dll
- QtSql4.dll (siehe oben)
- QtNetwork4.dll

- Microsoft Visual C++ 2008 SDK

- qt_de.qm: notfalls per "lrlease" aus "qt_de.ts" generieren



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

Die meisten Abhängigkeiten werden automatisch erkannt. Ein paar muss man noch
händisch hinzufügen.

So sollte das MSI-Projekt in etwa aussehen:


Per "View"->"File System"
Application Folder
- sqldrivers (Folder)
  - qsqlodbc4.dll
  - qsqlpsql4.dll
- Primary Ouput from sctime (Active)
- qt_de.qm
- QtCore4.dll
- QtGui4.dll
- QtNetwork4.dll
- QtSql4.dll
- QtXml4.dll
- sc32pixel.ico

User's Programs Menu
- Sctime (Shortcut)



zur Kontrolle die Ansicht im "Solution Explorer"

sctime-setup
- "Detected Dependencies":
  - policy_9_0_Microsoft_VC90_CRT_x86.msm
  - QtCore4.dll
  - QtGui4.dll
  - QtNetwork4.dll
  - QtSql4.dll
  - QtXml4.dll
- Primary Ouput from sctime (Active)
- Microsoft_VC90_CRT_x86.msm
- qsqlodbc4.dll
- qsqlpsql4.dll
- sc32pixel.ico
- qt_de.qm


Mit F4 auf der Projekt-Eintrag erhält man das "Properties Window", das sich
komplett von dem Fenster unterscheidet, das man mit Rechtsklick->"Properties"
erhält.

