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

#ifndef KONTOTREE_ITEM_H
#define KONTOTREE_ITEM_H

#include <QTreeWidgetItem>
#include "sctimexmlsettings.h"
class QTreeWidget;
class QPaintEvent;
class QString;

class KontoTreeItem: public QTreeWidgetItem
{
  public:
    enum Columns { COL_ACCOUNTS, COL_TYPE, COL_PSP, COL_ACTIVE, COL_TIME, COL_ACCOUNTABLE, COL_COMMENT };
    const static int NUM_COLUMNS=7;

    KontoTreeItem (QTreeWidget * parent, SCTimeXMLSettings::DefCommentDisplayModeEnum displaymode);
    KontoTreeItem (QTreeWidgetItem * parent, SCTimeXMLSettings::DefCommentDisplayModeEnum displaymode);


    KontoTreeItem (QTreeWidget * parent, SCTimeXMLSettings::DefCommentDisplayModeEnum displaymode, QString accountname); 

    KontoTreeItem (QTreeWidgetItem * parent, SCTimeXMLSettings::DefCommentDisplayModeEnum displaymode, QString accountname); 

    void setBoldAccount(bool bold);
    
    void setMicroAccount(bool microaccount);

    void setHasSelectableMicroAccounts(bool hasselectablema);

    void setGray();

    void setBgColor(const QColor bgColor);

    QString getLabel1();

    KontoTreeItem* nextSibling( );

  protected:
    void paintEvent(QPaintEvent *event);

  private:
    bool isBoldAccount;
    bool isMicroAccount;
    bool hasSelectableMicroAccounts;
    SCTimeXMLSettings::DefCommentDisplayModeEnum displaymode;
    bool isGray;
    SCTimeXMLSettings* settings;
    QColor m_bgColor;
};

#endif

