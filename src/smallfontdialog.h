/****************************************************************************
**
** Copyright (C) 1992-2006 Trolltech AS. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SMALLFONTDIALOG_H
#define SMALLFONTDIALOG_H

#include <QDialog>
#include <QFont>
#include <QComboBox>
#include <QFontDatabase>
#include <QLabel>
#include <QGroupBox>
#include <QBoxLayout>

class QFontListView;

class SmallFontDialog: public QDialog
{
    Q_OBJECT    

public:
    static QFont getFont(bool *ok, const QFont &def, QWidget* parent=0);
    static QFont getFont(bool *ok, QWidget* parent=0);
    static void parseFontName(const QString &name, QString &foundry, QString &family);

private:
    static QFont getFont(bool *ok, const QFont *def, QWidget* parent=0);

    explicit SmallFontDialog(QWidget* parent=0, bool modal=false, Qt::WFlags f=0);
    ~SmallFontDialog();

    QFont font() const;
    void setFont(const QFont &font);

    bool eventFilter(QObject *, QEvent *);

    void updateFamilies();
    void updateSizes();

private slots:
    void sizeChanged(const QString &);
    void familyHighlighted(int);        
    void sizeHighlighted(int);
    void updateSample();
    
private:
    QLabel * familyAccel;
    QLineEdit * familyEdit;
    QFontListView * familyList;

    QLabel * sizeAccel;
    QLineEdit * sizeEdit;
    QFontListView * sizeList;

    QGroupBox * sample;
    QLineEdit * sampleEdit;
    
    QPushButton * ok;
    QPushButton * cancel;

    QBoxLayout * buttonLayout;
    QBoxLayout * sampleLayout;
    QBoxLayout * sampleEditLayout;

    QFontDatabase fdb;

    QString       family;    
    QString       style;
    int           size;

    bool smoothScalable;
    
};

#endif // QT_NO_FONTDIALOG

