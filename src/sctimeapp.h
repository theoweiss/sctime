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

#ifndef SCTIMEAPP_H
#define SCTIMEAPP_H

#include <QApplication>

#ifdef WIN32
#include <windows.h>
#endif

class Lock;
class TimeMainWindow;
class SignalHandler;

class SctimeApp : public QApplication {
Q_OBJECT

public:
    SctimeApp(int &argc, char **argv);
    virtual ~SctimeApp();
#ifdef WIN32
    // catch suspend and resume events and handle them by calling "suspend()" or "resume()"
    virtual bool winEventFilter(MSG * msg, long * result);
#endif

   void init(Lock* lock, QStringList& dataSourceNames, const QString& zeitkontenfile,
             const QString& bereitschaftsfile, const QString& specialremunfile, const QString& offlinefile, const QString& logfile);
public slots:
   void cleanup(); 
private:
   Lock *m_lock;
   TimeMainWindow* mainWindow;
#ifndef WIN32
   SignalHandler* term;
   SignalHandler* hup;
   SignalHandler* int_;
   SignalHandler* cont;
#endif
};

#endif