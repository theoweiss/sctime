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


* IDE

  Das Layout der Fenster ist mit "qtcreator" (SuSE-Paket "qt-creator")
  erstellt. Zumindest unter Linux ist "qtcreator" auch eine gute
  C++-IDE. Unter Windows gab's irgendwelche Probleme beim Build. Außerdem
  braucht man unter Windows sowieso Visual Studio, um ".msi"-Pakete zu bauen.

  Achtung: Man kann nicht unter Windows und UNIX gleichzeitig auf dem gleichen
  Verzeichnis mit "qtcreator" arbeiten. Selbst abwechselndes Arbeit auf einem
  Verzeichnis ist nicht schön, weil sich die unterschiedlichen Versionen die
  Datei "sctime.pro.user" gegenseitig überschreiben.


* Voraussetzungen fürs Übersetzen

  Qt-Entwicklerpaket >= 4.4

** Hinweise für UNIX

   Die Qt-Entwicklerpakete heißen bei SuSE "libqt4-devel" und "libqt4-devel-doc".

  Die UNIX-Versionen werden zur Laufzeit mit dynamischen Qt-Bibliotheken
  gelinkt. Wenn man mit Qt 4.7 compiliert, läuft das Programm nicht auf
  Rechnern mit Qt 4.4. Deswegen sollte man die "sctime"-Pakete auf einem
  Rechner bauen, der den ältsten Rechnern der Benutzer gleicht.

  Entwickeln kann man woanders, auch mit einem neueren Qt. Wenn man dann
  Qt-Methode benutzt, die es in der alten Version noch nicht gibt, bemerkt man
  das erst beim Build. Doch selbst, wen man nur alte Methoden verwendet, gilt
  die obige Warnung vor dem Compilieren auf neuen Rechner.

  Man muss nur die Hinweise in
  der Qt-Doku beachten, welche Methode seit wann verfügbar ist. (Und selbst
  dann gilt die obige Warnung.)

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

  Mit QTDIR ist im folgenden das Installationsverzeichnis von Qt gemeint.

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

** Build

   Vorwarnung: Visual Studio verträgt sich nicht immer gut mit
   Samba-Freigaben. Diese Probleme erspart sich womöglich, wer unter Windows
   auf "C:" arbeitet.

   Zuerst bitte die Versionsbezeichung in "src\src.pro" setzen!

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

    Zum Testen muss man dass Programm meist erst mal packen und dann
    installieren. Die ausführbare Datei (sctime.exe) direkt vom
    Netzwerklaufwerk auszuführen, läuft Windows-Sicherheitsrichtlinien zuwider
    -- und erst recht, wenn dann auch noch Bibliotheken (.dll) nachgeladen
    werden sollen. Deswegen:

** MSI-Paket bauen

*** Vorbemerkungen

    Wir erstellen jetzt ein "Setup-Projekt" (.vcdproj) in Visual
    Studio 2008. Leider scheint es so, als könne man Projektdateien nicht so
    leicht untereinander austauschen.  Hier ein schönes HOWTO fürs
    prinzipielle Vorgehen, auch wenn die folgende Anleitung den Anspruch hat,
    ausreichend ausführlich zu sein.

   http://appdeveloper.intel.com/en-us/article/how-creating-your-msi-installer-using-visual-studio-2008

   Vorwarnung: Bei jedem neuen Aufruf von "qmake" ist das Setup-Projekt wieder
   draußen, doch dann kann man es mit Rechtsklick auf die Solution wieder
   hinzufügen: "Add Project -> "Existing Project"

*** Projekt anlegen

    Im "Solution Explorer" auf die "Solution" Rechtsklick -> Add -> New
    Project...

    Other Project Types -> "Setup Project", Name: "sctime" -> OK
    Es erscheint ein neues Projekt im "Solution Explorer".

*** Automatische Abhängigkeiten hinzufüngen

    Rechtsklick aufs Setup-Projekt -> Add -> Project Output... -> OK.
    Jetzt erscheinen einige ".msm"- und ".dll"-Einträge.

*** Zusätzliche Dateien einpacken

    Im "Qt Command Prompt".
    > lrelease QTDIR\translations\qt_de.ts
    Dadurch entsteht die Datei qt_de.qm.

    In der "File System"-Ansicht (Menü View -> Editor -> File System)
    Klick auf "Application Folder", dann Rechtsklick -> "Add" ->
    "File..." (oder per Drag and Drop aus dem Explorer) folgende Dateien
    hinzufügen:

    - QTDIR\translations\qt_de.qm

    - PROJDIR\pics\Uhr.ico

    - QTDIR\plugins\sqldrivers\qsqlpsql4.dll

    - QTDIR\plugins\sqldrivers\qsqlodbc4.dll

    Achtung: Bei den DLLs nicht die Debug-Versionen einpacken! Diese wären am
    "d" vor ".dll" zu erkennen.

    Danach sollte das Setup-Projekt im Solution Explorer so aussehen, wobei
    die genaue Reihenfolge keine Rolle spielt:

    sctime
    - "Detected Dependencies":
    -- Microsoft_VC90_CRT_x86.msm
    -- policy_9_0_Microsoft_VC90_CRT_x86.msm
    -- QtCore4.dll
    -- QtGui4.dll
    -- QtNetwork4.dll
    -- QtSql4.dll
    -- QtXml4.dll
    - Primary Ouput from sctime (Active)
    - Uhr.ico
    - qt_de.qm
    - qsqlodbc4.dll
    - qsqlpsql4.dll

*** Shortcut anlegen

    In der "File System"-Ansicht (Menü View -> Editor -> File System)
    Klick auf "Application Folder".
    Dann Rechtsklick auf "Primary Output..." -> "Create Shortcut...".
    Dem "Shortcut" geben wir den Namen "sctime".
    Danach den "Shortcut" nach "User's Programs Menu" verschieben.
    Dann Rechtsklick auf den Shortcut -> "Properties Window"

    Bei "Description" eingeben: "Arbeitszeiterfassung"

    Dann das "Icon" wählen: "(Browse...)".
    Es erscheint ein Dialog mit Titel "Icon".
    Dort "Browse..." anklicken.
    Es erscheint ein neuer Dialog mit Titel "Select Item in Project".
    Bei "Look in": "Application Folder" auswählen.
    Dann "Uhr.ico" wählen. (Wenn nicht verfügbar: zurück zu "Zusätzliche
    Dateien einpacken")
    Jetzt alle Dialog mit "OK" schließen.

*** Weitere Einstellungen fürs Setup-Projekt

    Jetzt im Solution Explorer nochmal das Setup-Projekt auswählen, dann F4.
    Dadurch erscheinen die "sctime Deployment Project Properties".
    AddRemoveIcons: wie bei Abschnitt "Shortcut"
    Description: Arbeitszeiterfassung
    Localization: German
    Manufacturer: science + computing ag
    RemovePreviousVersions: True
    SupportPhone: TELEFONNUMMER
    SupportUrl: https://scwiki.science-computing.de/twiki/bin/view/SIinfo/SctimeDokumentation
    UpgradeCode:  {C0A35EA4-7B53-4B09-B95F-D638A2FB836C}
    Version: VERSIONSNUMMER

    Wenn man die Versionsnummer ändert, stellt Visual Studio die Frage "It is
    recommand that the ProductCode be changed if you change the version. Do you
    want to do this?". Hier kann man "Yes" klicken.

    Die Versionsnummer hier kann leider nicht automatisch aus "src\src.pro"
    geholt werden. Man muss sie deswegen hier händisch eintragen.

    Durch einen einheitlichen "UpgradeCode" werden andere (alte) Versionen bei
    der Installation einer neuen Version entfernt.

*** "Silent Install"

    Bei der Installation sollen keine überflüssigen Begrüßungen und Fragen
    kommen. Dazu: Solution Explorer, Rechtsklick auf Setup-Projekt, dann
    "View" -> "User Interface".  Dort unterhalb von "Install" -> "Start",
    "Progress" und "End" alles löschen (wie z.B. "Welcome"). Bei
    "Administrative Install" können wir alles belassen.

*** Build

    Im "Solution Explorer" Rechtsklick aufs Setup-Projekt -> "Build".

*** Installieren

    Im "Solution Explorer" Rechtsklick aufs Setup-Projekt -> "Install".
    Nach ein paar Sekunden erscheint "sctime" im Startmenü.

    Alternativ die erstellte .msi-Datei anklicken.
