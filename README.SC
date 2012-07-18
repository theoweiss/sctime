-*- mode: org; coding: utf-8 -*-

sctime ist eine Qt-basierte GUI zur Erfassung von Arbeitszeit und
Bereitschaftszeiten.

Dieser Text enthält Informationen für Entwickler und ein wenig für Administratoren.

* Archiv

  Das git-Archiv von "sctime" ist auf Sourceforge.
  https://sourceforge.net/projects/sctime/ -> "Code" -> "Git"

  Kurzfassung: für reinen Lesezugriff:
  $ git clone git://sctime.git.sourceforge.net/gitroot/sctime/sctime


* Weitere Dokumentation

  - im Unterverzeichnis "help/": die Online-Hilfe

  - im Unterverzeichnis "doc/"

  - die offizielle Projektseite: https://sourceforge.net/projects/sctime/


* Voraussetzungen fürs Übersetzen

  Qt-Entwicklerpaket >= 4.4 (SuSE-Pakete "libqt4-devel" und "libqt4-devel-doc")

** Hinweise für UNIX

  Die UNIX-Versionen werden zur Laufzeit mit dynamischen Qt-Bibliotheken
  gelinkt. Wenn man mit Qt 4.7 compiliert, läuft das Programm nicht auf
  Rechnern mit Qt 4.4. Deswegen sollte man die "sctime"-Pakete auf einem
  Rechner bauen, der den ältsten Rechnern der Benutzer. Entwickeln kann man
  woanders, auch mit einem neueren Qt. Man muss dann nur die Hinweise in der
  Qt-Doku beachten, welche Methode seit wann verfügbar ist.

** Zusätzliche Voraussetzungen fürs Übersetzen unter Windows

  - Unter Windows braucht man mindestens einen der beiden QT-SQL-Treiber
    "qsqlpsql.dll" oder "qsqlodbc.dll". Beide sind im kommerziellen
    Qt-Entwicklerpaket enthalten.

  - Microsoft Visual C++ 2008 SDK (kostenlos von Microsoft)

  - Visual Studio 2008

  - qt_de.qm: zur Internationalisierung de Standarddialoge.
    Wenn die Datei nicht im Qt-Distribution enthalten sind, muss man die Datei
    "qt_de.ts" suchen und sie per "lrlease qt_de.ts" übersetzen. "lrelease"
    ist ebenfalls im Qt-Entwicklerpaket enthalten.


* Abhängigkeiten zur Laufzeit

** Windows

   Wann man das QT-SQL-Plugin "qsqlpsql.dll" im ".msi" nicht mitliefert,
   versucht "sctime" "qsqlodbc.dll", und damit die Windows-ODBC-Datenquellen
   zu verwenden. Dann muss man dafür sorgen, dass Postgres-ODBC-Treiber
   installiert wird (gibt's auf der offiziellen Postgres-Seite zum Runterladen).

   Um das Programm laufen zu lassen, sollte man dann ein .msi-Paket bauen und
   installieren (s.u.)

   Wenn man stattdessen die ".exe"-Datei direkt starten will, dann muss man
   die Qt-Bibliotheken QtGui4.dll, QtXml4.dll, QtCore4.dll, QtSql4.dll und
   QtNetwork4.dll ins gleiche Verzeichnis kopieren. (mscvrt.dll?)  Außerdem
   muss man das "plugins"-Verzeichnis dorthin kopieren. Und Windows muss dem
   Laufwerk vertrauen, in dem die Bibliothken liegen. Das ist bei
   Netzwerklaufwerken problematisch.

** UNIX
   Zur Starten müssen die Qt-Laufzeitbibliotheken installiert sein (bei SuSE:
   Paket "libqt4"). Es gibt Probleme, wenn die Laufzeitbibliotheken älter sind
   als die auf dem Build-Rechner (=> alten Buildrechner verwenden!).

   Um die Liste der Zeitkonten abzurufen, kann "sctime" unter UNIX die
   Kommandos "zeitkonten" und "zeitbereitls" verwenden. Vorzugsweise verwendet
   "sctime" jedoch eventuell installierte Module für den Datenbankzugriff
   ("qsqlpsql.so" oder Ähnliches). Diese Module muss dann der Admin
   installieren. Bei openSuSE heißt das Paket "libqt4-sql-postgresql".


* Allgemeines zum Build

  Projektverzeichnis (PROJDIR): Hier ist die Arbeitskopie des Archivs.
  Buildverzeichnis (BUILDDIR): Hier (unter darunter) kompiliert man.

  Das Buildverzeichnis muss nicht das Projektverzeichnis sein -- und sollte es
  auch nicht sein, wenn man für mehrere Architekturen baut. Wenn PROJDIR !=
  BUILDDIR spricht man übrigens von einem "out of source build" oder "shadow
  build".

  Qt hat kein eigenes Build-System. Stattdessen muss man wie bei "cmake" mit
  "qmake" erst mal eine Eingabedatei fürs eigene Buildsystem (make, .sln, ..)
  erstellen.


* Build auf UNIX

  Wie schon erwähnt, sollte man auf einem eher alten Rechner bauen, damit das
  Programm sowohl auf alten als auch auf neuen Rechner funktioniert.

  Zuerst sollte man die Versionsbezeichnung in "src/src.pro" kontrollieren.

  Am Besten legt man sich dann für den Build auf jeder Architektur ein neues
  Verzeichnis an. Dort gibt man ein:

  $ qmake -r $PROJDIR/sctime.pro  # => Makefile erstellt
  $ gmake -j4
  $ src/sctime  # testen
  $ cd src
  $ $PROJDIR/extra/unix/packen  # => P4-Paket erstellt


* Build für Windows mit Visual Studio 2008

** Übersetzen

   Wie immer: Erst mal die Versionsbezeichung in "src\src.pro" setzen!

   Visual Studio verträgt sich nicht immer gut mit Samba-Freigaben. Diese
   Probleme erspart sich, wer unter Windows auf "C:" arbeitet.

   Man startet den "Qt Command Prompt", wechselt ins Buildverzeichnis (das man
   ggf. vorher anlegt) und gibt ein:

   $ qmake -tp vc -r PROJDIR\sctime.pro
   $ sctime.sln   => Visual Studio startet


   Danach unter "Build"/"Configuration Manager" auf "Release" umstellen! Für
   die Konfiguration "Debug" braucht man nämlich die Debug-Bibliotheken, die
   vielleicht nicht installiert sind. Das muss man nach jedem "qmake"-Lauf
   wiederholen!

   Einzeilschritt-Abarbeitung (Debuggen) scheint nicht möglich: "Access
   violation"

   Wenn man bis hierher testen will: "Build Solution"

** MSI-Paket bauen

   Wenn PROJDIR != BUILDDIR: %PROJDIR%\sctime-setup nach %BUILDDIR% kopieren

   Dann, beim ersten Mal und jedes Mal nach "qmake":
   Dann in Visual Studio "File" -> "Add" -> "Existing Project": "sctime-setup"
   auswählen => Fenster "Deployment Project Properties". Hier die
   Versionsbezeichnung setzen!

   Dann "Build Solution" (Ctrl-Shift-B) => sctime-setup.msi.

   Schnell installieren kann man das Paket im "Solution Explorer" mit
   Rechtsklick -> "Install" auf "sctime-setup."

** MSI-Paket selbst bauen

   Wenn der oben gezeigte Ansatz nicht funktioniert, muss man sich in
   "Visual Studio" selbst ein "Setup Project" zusammenklicken. Hier ein
   schönes HOWTO fürs prinzipielle Vorgehen:

   http://appdeveloper.intel.com/en-us/article/how-creating-your-msi-installer-using-visual-studio-2008

   Bei jedem neuen Aufruf von "qmake" ist das Setup-Projekt wieder draußen, doch
   dann kann man es mit Rechtsklick auf die Solution wieder hinzufüngen:
   "Add Project -> "Existing Project"

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
  - Uhr.ico

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
  - Uhr.ico
  - qt_de.qm


    Mit F4 auf der Projekt-Eintrag erhält man die "Deployment Project
    Properties". Das unterscheidet sich sehr von dem nutzlosen Fenster, das
    man mit Rechtsklick->"Properties" erhält.

    Zum Testen muss man dass Programm meist erst mal packen und dann
    installieren. Direktes Ausführen vom Netzwerklaufwerk läuft manchen
    Windows-Sicherheitsrichtlinien zuwider -- zumindest, wenn dann auch noch
    Bibliotheken nachgeladen werden sollen.

* IDE

  Die GUIs sind mit "qtcreator" (SuSE-Paket "qt-creator") erstellt. Zumindest
  unter Linux ist "qtcreator" insgesamt eine gute C++-IDE. Unter Windows gab's
  irgendwelche Probleme.

  Achtung: Man kann nicht unter Windows und UNIX gleichzeitig auf dem gleichen
  Verzeichnis mit "qtcreator" arbeiten. Selbst abwechselndes Arbeit auf einem
  Verzeichnis ist nicht schön, weil sich die unterschiedlichen Versionen die
  Datei "sctime.pro.user" gegenseitig überschreiben.

