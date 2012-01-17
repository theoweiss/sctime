sctime
Version 0.61
01.10.2010

sctime provides a graphical interface which allows users to record the times used for
specific projects.

DEVELOPER:
Florian Schmitt f.schmitt@science-computing.de

REPOSITORY:
This project uses CVS for version control. Use the following commands
to check it out:
cvs -d:pserver:anonymous@cvs.sourceforge.net:/cvsroot/sctime login
cvs -z3 -d:pserver:anonymous@cvs.sourceforge.net:/cvsroot/sctime co sctime


REQUIREMENTS:
The zeittools are required to access the database.
A qt4 Library is required.

BUILDING:
Go to the src folder and run "qmake".
Please check the paths in the apropriate
Makefile before running make. Then run make.


INSTALL:
All you need is the executable sctime, which you may copy
to a directory in your PATH variable.

TESTING:
To make really use of this project, you need to have two scripts
named "zeit" and "zeitkonten". By the script "zeit" the data can be checked into a database.
As a "zeit" script is not yet available under GPL, so you'll probably have to do without
database functionality (as long as you don't write your own wrapper).
The script "zeitkonten" should deliver a list of accounts, with each line consisting of the
following information, which is separated by pipe symbols (|):
account|subaccount|department|accounting-nr|person responsible|type|comment

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



