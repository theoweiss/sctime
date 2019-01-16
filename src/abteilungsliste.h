/*

    Copyright (C) 2018 science+computing ag
       Authors: Florian Schmitt et al.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#ifndef ABTEILUNGSLISTE_H
#define  ABTEILUNGSLISTE_H

#include <QDateTime>
class QColor;

// nicht als Vorwärtsdeklarationen möglich
#include "kontoliste.h"
#include "unterkontoliste.h"
#include "eintragsliste.h"
#include "datasource.h"

#define IS_DISABLED 8
#define IS_IN_DATABASE 4
#define IS_CLOSED 2
#define FLAG_MODE_OVERWRITE 0
#define FLAG_MODE_OR 1
#define FLAG_MODE_NAND 2
#define FLAG_MODE_XOR 3

class KontoDatenInfo;
class SpecialRemunTypeList;

/** AbteilungsListe ist eine map, die einem Abteilungsnamen eine KontoListe zuordnet.
 * Desweiteren finden sich hier Methoden zum Zugriff auf die zu darin gespeicherten
 * Abteilungen, Konten, Unterkonten und Eintraege.
 * Die urspruengliche Idee, war, den Aufbau der darunterliegenden Maps vor den Anwendenden
 * Klassen zu verbergen, und nur ueber die Strings auf die Konten zuzugreifen, denn
 * Strings lassen sich sehr einfach aus dem Listview extrahieren.
 * Leider hat das im Laufe der Zeit zu einigen Methoden in Abteilungsliste gefuehrt
 * die sich stark aehneln. Hier sollte vielleicht mal umgebaut und verallgemeinert werden, was aber
 * Aenderungen in allen Bereichen des Programms nach sich zieht.
 */

class AbteilungsListe: public std::map<QString,KontoListe>
{
  public:

    AbteilungsListe(const QDate& _datum, KontoDatenInfo* ki);

    AbteilungsListe(const QDate& _datum, AbteilungsListe* abtlist);

    bool findDepartment(KontoListe*&, const QString&);

    bool findKonto(KontoListe::iterator& itKo, KontoListe* &kontoliste, const QString& abteilung, const QString& konto);

    bool findUnterKonto(UnterKontoListe::iterator& itUk, UnterKontoListe* &unterkontoliste, const QString& abteilung, const QString& konto, const QString& unterkonto);

    bool findEintrag(EintragsListe::iterator& itEt, EintragsListe* &eintragsliste, const QString& abteilung, const QString& konto, const QString& unterkonto, int idx);

    bool findEntryWithSpecialRemunsAndComment(EintragsListe::iterator& itEt, EintragsListe* &eintragsliste, int & idx, const QString& abteilung, const QString& konto, const QString& unterkonto, const QString& comment, const QSet<QString>& specialRemuns);

    QString findAbteilungOfKonto(const QString& konto);

    KontoListe* insertAbteilung(const QString& abteilung);

    UnterKontoListe* insertKonto(const QString& abteilung,const QString& konto);

    EintragsListe* insertUnterKonto(const QString& abteilung,const QString& konto,const QString& unterkonto);

    int insertEintrag(const QString& abteilung, const QString& konto, const QString& unterkonto, int idx=-1);

    void deleteEintrag(const QString& abteilung, const QString& konto, const QString& unterkonto, int idx);

    bool getEintrag(UnterKontoEintrag& eintrag, const QString& abteilung, const QString& konto, const QString& unterkonto, int idx);

    bool setEintrag(const QString& abteilung, const QString& konto, const QString& unterkonto, int idx, const UnterKontoEintrag& uk, bool regular=false);

    void setDescription(const QString& abteilung, const QString& konto, const QString& unterkonto, const DescData& descData);

    DescData getDescription(const QString& abteilung, const QString& konto, const QString& unterkonto);

    void setBgColor(QColor bgColor, const QString& abteilung, const QString& konto="", const QString& unterkonto="");
    QColor getBgColor(const QString& abteilung, const QString& konto="", const QString& unterkonto="");
    void unsetBgColor(const QString& abteilung, const QString& konto="", const QString& unterkonto="");
    bool hasBgColor(const QString& abteilung, const QString& konto="", const QString& unterkonto="");

    bool setKommentar(const QString& abteilung, const QString& konto, const QString& unterkonto, int idx, const QString& kommentar);

    bool setSekunden(const QString& abteilung, const QString& konto, const QString& unterkonto, int idx, int sekunden, bool regular=false);

    bool setSekundenAbzur(const QString& abteilung, const QString& konto, const QString& unterkonto, int idx, int sekundenabzur);

    void minuteVergangen(bool abzur);

    void changeZeit(const QString& Abteilung,const QString& Konto,const QString& Unterkonto,int Eintrag, int delta, bool abzurOnly=false, bool regular=false, bool workedOnly=true);

    bool setEintragFlags(const QString& abteilung, const QString& konto, const QString& unterkonto, int idx, int flags, int mode=FLAG_MODE_OVERWRITE);

    bool setUnterKontoFlags(const QString& abteilung, const QString& konto, const QString& unterkonto, int flags, int mode=FLAG_MODE_OVERWRITE);

    bool setKontoFlags(const QString& abteilung, const QString& konto, int flags, int mode=FLAG_MODE_OVERWRITE);

    bool setAbteilungFlags(const QString& abteilung, int flags, int mode=FLAG_MODE_OVERWRITE);

    //void setAllEintragFlags(int flags);

    void setAllEintragFlagsInKonto(const QString& abteilung, const QString& konto, int flags);

    void setAllEintragFlagsInUnterKonto(const QString& abteilung, const QString& konto, const QString& unterkonto, int flags);

    void moveEintragPersoenlich(const QString& abteilung, const QString& konto, const QString& unterkonto, int idx, bool persoenlich);

    void moveUnterKontoPersoenlich(const QString& abteilung, const QString& konto, const QString& unterkonto, bool persoenlich);

    void moveKontoPersoenlich(const QString& abteilung, const QString& konto, bool persoenlich);

    int getEintragFlags(const QString& abteilung, const QString& konto, const QString& unterkonto, int idx);

    int getAbteilungFlags(const QString& abteilung);

    int getKontoFlags(const QString& abteilung, const QString& konto);

    int getUnterKontoFlags(const QString& abteilung, const QString& konto, const QString& unterkonto);

    void getAktiv(QString& abteilung, QString& konto, QString& unterkonto, int& idx);

    void setAsAktiv(const QString& abteilung, const QString& konto, const QString& unterkonto, int idx);
    bool ukHatMehrereEintrage(const QString& abteilung, const QString& konto, const QString& unterkonto, int idx);

    bool isAktiv(const QString& abteilung, const QString& konto, const QString& unterkonto, int idx);

    bool isPersoenlich(const QString& abteilung, const QString& konto, const QString& unterkonto, int idx);

    void getGesamtZeit(int& sek, int& sekabzur);

    int getZeitDifferenz();

    void setZeitDifferenz(int);

    const QDate& getDatum();

    void setDatum(const QDate& datum);

    void clearKonten();

    void clearDefaultComments();

    void reload(const DSResult &data);

    bool kontoDatenInfoConnected();

    void getUnterKontoZeiten(const QString& abteilung, const QString& konto,
                                         const QString& unterkonto, int& sek, int& sekabzur);

    bool checkInState();

    void setCheckInState(bool state);
    bool kontoDatenInfoSuccess;
    
    const QList<QString>& getGlobalSpecialRemunNames() const;
    
    void setGlobalSpecialRemunNames(const QList<QString>& srtl);
    
    const SpecialRemunTypeMap& getSpecialRemunTypeMap() const;
    
    void setSpecialRemunTypeMap(const SpecialRemunTypeMap& srtm);

    bool overTimeModeActive();

    void setOverTimeModeState(bool active, const QString& srname);

    bool overTimeModeState(const QString& srname);
    
    QSet<QString> getActiveOverTimeModes();
    

  private:
    SpecialRemunTypeMap m_specialRemunTypeMap;
    QList<QString> m_globalSpecialRemunNames;
    QString aktivAbteilung, aktivKonto, aktivUnterkonto;
    KontoDatenInfo* kontoDatenInfo;
    int aktivEintrag;
    int zeitDifferenz;
    bool checkedIn;
    QDate datum;
    QSet<QString> m_activeOverTimeModes;

};


#endif
