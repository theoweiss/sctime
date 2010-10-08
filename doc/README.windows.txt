Damit sctime die Zeit-Datenbank auslesen kann, muss
ODBC entsprechend eingerichtet sein.
Dazu muss zunaechst ein Treiber von
ftp://ftp.postgresql.org/pub/odbc/versions/full
installiert werden.
Anschliessend muss die ODBC-Datenquelle hinzugefuegt
werden.
Unter Windows XP funktioniert das folgendermassen:
In der Systemsteuerung Verwaltung auswaehlen. Dort
Datenquellen(ODBC) oeffnen.
Hier eine neue Benutzer-DSN fuer den PostgreSQL-Treiber
hinzufuegen, mit den folgenden Eigenschaften:

Data Source: zeit
Database: zeit
Server: dabaserv
Port: 5432

Als Username ist der Loginname, und als Passwort der
Inhalt der Datei "H:\.Zeit" einzutragen.

Bei den Optionen sollte man unter DataSource
sicherheitshalber noch "ReadOnly" aktivieren.

