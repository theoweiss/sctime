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

#ifndef JSONREADER_H
#define JSONREADER_H

#include <QString>
#include <QException>

#include <QJsonDocument>
#include "datasource.h"

class JSONReaderException : public QException
{
public:
    void raise() const override { throw *this; }
    JSONReaderException *clone() const override { return new JSONReaderException(*this); }
};

class JSONReaderBase
{
public:
  const static int INVALIDDATA=-1;
  virtual ~JSONReaderBase() {};
  virtual int loadDataNewerThan(int version);
  virtual QByteArray getByteArray()=0;
  virtual QJsonDocument& getData();
private:
  QJsonDocument data;
  int currentversion;
protected:
  JSONReaderBase();
};

class JSONReaderFile: public JSONReaderBase
{
public:
  JSONReaderFile(const QString& path);
  virtual QByteArray getByteArray();
  virtual ~JSONReaderFile() {};
private:
  const QString path;
};

class JSONReaderCommand: public JSONReaderBase
{
public:
  const static int INVALIDDATA=-1;
  JSONReaderCommand(const QString& command, QObject* parent);
  virtual QByteArray getByteArray();
  virtual ~JSONReaderCommand() {};
private:
  const QString command;
  QObject* parent;
};

class JSONSource: public Datasource
{
public:
  JSONSource(JSONReaderBase *jsonreader);
  virtual ~JSONSource() {};
  virtual bool read(DSResult* const result);
private: 
  int currentversion;
protected:
  JSONReaderBase* jsonreader;
  void appendStringToRow(QStringList& row, const QJsonObject& object, const QString& field);
  virtual bool convertData(DSResult* const result)=0;
};

class JSONAccountSource: public JSONSource
{
public:
  JSONAccountSource(JSONReaderBase *jsonreader);
  virtual ~JSONAccountSource() {};
protected:
  virtual bool convertData(DSResult* const result);
};

class JSONSpecialRemunSource: public JSONSource
{
public:
  JSONSpecialRemunSource(JSONReaderBase *jsonreader);
  virtual ~JSONSpecialRemunSource() {};
protected:
  virtual bool convertData(DSResult* const result);
};

class JSONOnCallSource: public JSONSource
{
public:
  JSONOnCallSource(JSONReaderBase *jsonreader);
  virtual ~JSONOnCallSource() {};
protected:
  virtual bool convertData(DSResult* const result);
};

#endif // JSONREADER_H
