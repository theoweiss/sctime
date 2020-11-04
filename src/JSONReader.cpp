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

#include "datasource.h"

#include "JSONReader.h"
#include "globals.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QTextStream>
#include <QProcess>



JSONSource::JSONSource(JSONReaderBase *jsonreader)
  :Datasource(), currentversion(JSONReaderBase::INVALIDDATA), jsonreader(jsonreader) {}

void JSONSource::appendStringToRow(QStringList& row, const QJsonObject& object, const QString& field) {
  row.append(object[field].toString());
}

bool JSONSource::read(DSResult* const result) {
  currentversion=jsonreader->loadDataNewerThan(currentversion);
  if (currentversion==JSONReaderBase::INVALIDDATA)
    return false;
  return convertData(result);
}

/*
 * creates a new JSONSource-Object. JSONSource reads from aJSONReader Object and creates a flat table,
 * with the following columns:
 * tbd
 * 
 * this is neither efficient nor very elegant, but it fits into the existing mechanism. Perhaps it will
 * be refactored one day.
 * 
 */
JSONAccountSource::JSONAccountSource(JSONReaderBase *jsonreader)
  :JSONSource(jsonreader) {}
  
bool JSONAccountSource::convertData(DSResult* const result) {
  QJsonDocument doc=jsonreader->getData();
  QJsonObject data=doc.object()["AccountTree"].toObject();
  QJsonArray departments=data["Departments"].toArray();
  foreach (auto departmentVal, departments) {
    QJsonObject department=departmentVal.toObject();
    QJsonArray accounts=department["Accounts"].toArray();
    foreach (auto accountVal, accounts) {
      QJsonObject account=accountVal.toObject();
      QString respaccount1="";
      QString respaccount2="";
      QJsonArray resppers=account["ResponsiblePersons"].toArray();
      if (!resppers.isEmpty()) {
        respaccount1=resppers.takeAt(0).toString();
        if (!resppers.isEmpty()) {
          respaccount2=resppers.takeAt(0).toString();
        }
      }
      QJsonArray subaccounts=account["SubAccounts"].toArray();
      foreach (auto subaccountVal, subaccounts) {
        QJsonObject subaccount=subaccountVal.toObject();
        QStringList row;
        appendStringToRow(row,department,"Name");
        appendStringToRow(row,account,"CostCenter");
        appendStringToRow(row,account,"Name");
        row.append(respaccount1);
        row.append(respaccount2);
        appendStringToRow(row,account,"InvoicedUntil");
        appendStringToRow(row,account,"NoEntriesBefore");
        appendStringToRow(row,subaccount,"Name");
        resppers=subaccount["ResponsiblePersons"].toArray();
        if (!resppers.isEmpty()) {
          row.append(resppers.takeAt(0).toString());
          if (!resppers.isEmpty()) {
            row.append(resppers.takeAt(0).toString());
          }
          else {
            row.append("");
          }
        }
        else {
          row.append("");
	  row.append("");
        }
        appendStringToRow(row,subaccount,"Category");
        appendStringToRow(row,subaccount,"Description");
        appendStringToRow(row,subaccount,"PSP");
        QJsonArray specialremuns=subaccount["SpecialRemunerations"].toArray();
        QStringList srlist;
        foreach (auto specialremunVal, specialremuns) {
          srlist.append(specialremunVal.toString());
        }
        row.append(srlist.join(","));
        QJsonArray microaccounts=subaccount["MicroAccounts"].toArray();
        row.append("");
        if (microaccounts.size()>0) {
          foreach (auto microaccountVal, microaccounts) {
            QString microaccount=microaccountVal.toString();
            row[row.size()-1]=microaccount; // we need a row for each microaccount, so replace it on the row
            result->append(row);            // and insert the new row to the result
          }
        } else {
            result->append(row); // if there are no microaccounts insert row anyway.
        }      
      }
    }
  }
  return true;
}

JSONReaderBase::JSONReaderBase()
  : currentversion(INVALIDDATA) {}
  
QJsonDocument &JSONReaderBase::getData()
{
  return data;
}



int JSONReaderBase::loadDataNewerThan(int version)
{
  if ((version<=currentversion)&&(currentversion!=INVALIDDATA))
     return currentversion;
  currentversion=version+1;
  QByteArray byteData;
  try {
     byteData = getByteArray();
  } catch (JSONReaderException *e) {
    return INVALIDDATA;
  }
  data=QJsonDocument::fromJson(byteData);
  return currentversion;
}

JSONOnCallSource::JSONOnCallSource(JSONReaderBase *jsonreader)
  :JSONSource(jsonreader) {}
  
bool JSONOnCallSource::convertData(DSResult* const result) {
  QJsonDocument doc=jsonreader->getData();
  QJsonArray oncalltimes=doc.object()["OnCallTimes"].toArray();
  foreach (auto oncalltimeVal, oncalltimes) {
    QJsonObject oncalltime=oncalltimeVal.toObject();
    QStringList row;
    appendStringToRow(row,oncalltime,"Category");
    appendStringToRow(row,oncalltime,"Description");
    result->append(row);
  }
  return true;
}

JSONSpecialRemunSource::JSONSpecialRemunSource(JSONReaderBase *jsonreader)
  :JSONSource(jsonreader) {}
  
bool JSONSpecialRemunSource::convertData(DSResult* const result) {
  QJsonDocument doc=jsonreader->getData();
  QJsonArray specialremuns=doc.object()["SpecialRemunerations"].toArray();
  foreach (auto specialremunVal, specialremuns) {
    QJsonObject specialremun=specialremunVal.toObject();
    QStringList row;
    appendStringToRow(row,specialremun,"Category");
    appendStringToRow(row,specialremun,"Description");
    if (specialremun["IsGlobal"].isString()) {
      row.append(specialremun["IsGlobal"].toString());
    } else {
      if (specialremun["IsGlobal"].toInt()) {
        row.append("1");
      } else {
        row.append("0");
      }
    }
    result->append(row);
  }
  return true;
}


QByteArray JSONReaderFile::getByteArray()
{
  QFile loadFile(path);

  if (!loadFile.open(QIODevice::ReadOnly)) {
      trace(QObject::tr("Couldn't open json file %1.").arg(path));
      loadFile.close();
      throw(new JSONReaderException());
  }

  QByteArray byteData = loadFile.readAll();
  loadFile.close();
  return byteData;
}

JSONReaderFile::JSONReaderFile(const QString& _path): JSONReaderBase(), path(_path) {};

QByteArray JSONReaderCommand::getByteArray()
{
  QProcess* process = new QProcess(parent);
  process->start(command,QIODevice::ReadOnly);
  trace(QObject::tr("Running command: ") + command);
  if (!process->waitForFinished(-1)) {
    logError(QObject::tr("Cannot run command '%1': %2").arg(command).arg(process->error()));
    delete process;
    throw(new JSONReaderException());
  }
  if (process->exitCode()) {
    logError(QObject::tr("Command '%1' has non-zero exitcode: %s2").arg(command, process->exitCode()));
    delete process;
    throw(new JSONReaderException());
  }
  QByteArray byteData = process->readAllStandardOutput();
  delete process;

  return byteData;
}

JSONReaderCommand::JSONReaderCommand(const QString& _command, QObject* _parent): JSONReaderBase(), command(_command), parent(_parent) {};
