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

#include "textviewerdialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

TextViewerDialog::TextViewerDialog(QWidget* parent, const QString& title, const QString& name, bool plaintext_links): QDialog(parent) {
  setObjectName(name);
  setWindowTitle(title);
  QVBoxLayout *layout = new QVBoxLayout(this);
  m_browser = new QTextBrowser(this);
  m_browser->setOpenExternalLinks(true);
  layout->addWidget(m_browser);
  layout->addSpacing(7);
  QHBoxLayout* buttonlayout=new QHBoxLayout();
  buttonlayout->setContentsMargins(3,3,3,3);
  QPushButton * okbutton=new QPushButton( tr("OK"), this);
  buttonlayout->addStretch(1);
  buttonlayout->addWidget(okbutton);
  buttonlayout->addStretch(1);
  layout->addLayout(buttonlayout);
  layout->addSpacing(4);
  connect (okbutton, SIGNAL(clicked()), this, SLOT(close()));
  if (plaintext_links) {
    connect (m_browser, SIGNAL(sourceChanged(const QUrl&)), this, SLOT(sourceChanged(const QUrl&)));
  }
}

// try to persuade qt to show plaintext files not as html
void TextViewerDialog::sourceChanged(const QUrl &src) {
    // if we most probably have an html dont mess with the heuristics
    if (!src.fragment().isNull() || src.path().endsWith(".htm") || src.path().endsWith(".html")) {
        return;
    }
    QNetworkAccessManager nam;
    QNetworkRequest request(src);
    auto reply = nam.get(request);
    m_browser->setHtml(QString::null);
    m_browser->document()->clear();
    m_browser->setPlainText(reply->readAll());
}