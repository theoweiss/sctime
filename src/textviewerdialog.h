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

#ifndef TEXTVIEWERDIALOG_H
#define TEXTVIEWERDIALOG_H

#include <QDialog>
#include <QTextBrowser>

class TextViewerDialog: public QDialog {
    Q_OBJECT
public:
    TextViewerDialog(QWidget* parent, const QString& title, const QString& name, bool plaintext_links=false);
    QTextBrowser* browser() { return m_browser; };
private:
    QTextBrowser* m_browser;
public slots:
    void sourceChanged(const QUrl &src);
};

#endif