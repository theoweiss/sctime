#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QString>
#include <QList>
#include <QObject>
class QStringList;
#include <QSqlDatabase>

typedef QList<QStringList> DSResult;

class Datasource
{
public:
  Datasource();
  virtual bool read(DSResult* result) = 0;
  bool broken; // the datasource detected a permanent error
};

class DatasourceManager:public QObject
{
  Q_OBJECT
public:
  DatasourceManager();
  QList<Datasource*> sources;
  ~DatasourceManager();
public slots:
  virtual void start();
signals:
  void finished(const DSResult& data);
  void aborted();
};

class FileReader : public Datasource
{
public:
  FileReader(const QString &path, const QString& sep, int columns);
  virtual bool read(DSResult* const result);
  const QString path, sep;
  const int columns;
  };

class SqlReader : public Datasource
{
public:
  SqlReader(QSqlDatabase db, const QString &cmd);
  virtual bool read(DSResult* const result);
  const QString cmd;
  QSqlDatabase db;
};


#ifndef WIN32
class CommandReader : public Datasource
{
public:
  CommandReader(const QString &command, const QString& sep, int columns);
  virtual bool read(DSResult* const result);
  const QString command, sep;
  const int columns;
};
#endif

#endif // DATASOURCE_H
