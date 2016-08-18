#ifndef JSONREADER_H
#define JSONREADER_H

#include <QString>

#include <QJsonDocument>
#include "datasource.h"

class JSONReader 
{
public:
  const static int INVALIDDATA=-1;
  JSONReader(const QString& path);
  virtual ~JSONReader() {};
  virtual int loadDataNewerThan(int version);
  virtual QJsonDocument& getData();
private:
  QJsonDocument data;
  int currentversion;
  const QString path;
};

class JSONSource: public Datasource
{
public:
  JSONSource(JSONReader *jsonreader);
  virtual ~JSONSource() {};
  virtual bool read(DSResult* const result);
private: 
  int currentversion;
protected:
  JSONReader* jsonreader;
  void appendStringToRow(QStringList& row, const QJsonObject& object, const QString& field);
  virtual bool convertData(DSResult* const result)=0;
};

class JSONAccountSource: public JSONSource
{
public:
  JSONAccountSource(JSONReader *jsonreader);
  virtual ~JSONAccountSource() {};
protected:
  virtual bool convertData(DSResult* const result);
};

class JSONSpecialRemunSource: public JSONSource
{
public:
  JSONSpecialRemunSource(JSONReader *jsonreader);
  virtual ~JSONSpecialRemunSource() {};
protected:
  virtual bool convertData(DSResult* const result);
};

class JSONOnCallSource: public JSONSource
{
public:
  JSONOnCallSource(JSONReader *jsonreader);
  virtual ~JSONOnCallSource() {};
protected:
  virtual bool convertData(DSResult* const result);
};

#endif // JSONREADER_H
