/*
    Copyright (C) 2018 science+computing ag
       Authors: Johannes Abt et al.

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

#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QString>
#include <QList>
#include <QObject>
#include <QSqlDatabase>
class QStringList;

typedef QList<QStringList> DSResult;

/* represents a way of reading data (a list of a list of strings); the base class of all data sources. */
class Datasource
{
public:
  /* false: error; */
  virtual bool read(DSResult* const result) = 0;
  /* the datasource detected a permanent error */
  bool broken;
  virtual ~Datasource() {};
  virtual QString toString()=0;
protected:
  Datasource();
};

class DatasourceManager:public QObject
{
  Q_OBJECT
public:
  /* name: identifier for message */
  DatasourceManager(const QString& name);
  const QString name;
  QList<Datasource*> sources;
  ~DatasourceManager();
public slots:
  virtual void start();
signals:
  /* successfully finished reading */
  void finished(const DSResult& data);
  /* no datasource provided data */
  void aborted();
};

class FileReader : public Datasource
{
public:
  FileReader(const QString &path, const QString& columnSeparator, int columns);
  virtual ~FileReader() {};
  virtual bool read(DSResult* const result);
  const QString path, sep;
  const int columns;
  virtual QString toString() {return "FileReader_"+path;};
  };

class SqlReader : public Datasource
{
public:
  SqlReader(QSqlDatabase db, const QString &cmd);
  virtual ~SqlReader() {};
  virtual bool read(DSResult* const result);
  const QString cmd;
  QSqlDatabase db;
  virtual QString toString() {return "SqlReader_"+cmd;};
};


#ifndef WIN32
class CommandReader : public Datasource
{
public:
  CommandReader(const QString &command, const QString& columnSeparator, int columns);
  virtual ~CommandReader() {};
  virtual bool read(DSResult* const result);
  const QString command, sep;
  const int columns;
  virtual QString toString() {return "CommandReader_"+command;};
};
#endif

#endif // DATASOURCE_H
