#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QString>
#include <QList>
#include <QObject>
#include <QSqlDatabase>
class QStringList;

typedef QList<QStringList> DSResult;

/* represents a way of reading data (a list of a list of strings) */
class Datasource
{
public:
  /* false: error; */
  virtual bool read(DSResult* const result) = 0;
  /* the datasource detected a permanent error */
  bool broken;
  virtual ~Datasource() {};
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
  };

class SqlReader : public Datasource
{
public:
  SqlReader(QSqlDatabase db, const QString &cmd);
  virtual ~SqlReader() {};
  virtual bool read(DSResult* const result);
  const QString cmd;
  QSqlDatabase db;
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
};
#endif

#endif // DATASOURCE_H
