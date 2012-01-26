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

#include "smallfontdialog.h"

#include <QDialog>
#include "qevent.h"
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLayout>
#include <QGroupBox>
#include <QLabel>
#include <QApplication>
#include <QFontDatabase>
#include <QStyle>
#include <QFont>
#include <QValidator>
#include <QStringListModel>
#include <QListView>
#include <QHeaderView>

class QFontListView : public QListView
{
    Q_OBJECT
public:
    QFontListView(QWidget *parent);
    QStringListModel *model() const {
        return static_cast<QStringListModel *>(QListView::model());
    }
    void setCurrentItem(int item) {
        QListView::setCurrentIndex(static_cast<QAbstractListModel*>(model())->index(item));
    }
    int currentItem() const {
        return QListView::currentIndex().row();
    }
    int count() const {
        return model()->rowCount();
    }
    QString currentText() const {
        int row = QListView::currentIndex().row();
        return row < 0 ? QString() : model()->stringList().at(row);
    }
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) {
        QListView::currentChanged(current, previous);
        if (current.isValid())
            emit highlighted(current.row());
    }
    QString text(int i) const {
        return model()->stringList().at(i);
    }
signals:
    void highlighted(int);
};

QFontListView::QFontListView(QWidget *parent)
    : QListView(parent)
{
    setModel(new QStringListModel(parent));
    setEditTriggers(NoEditTriggers);
}

/*!
  \class QFontDialog qfontdialog.h
  \ingroup dialogs
  \mainclass
  \brief The QFontDialog class provides a dialog widget for selecting a font.

  The usual way to use this class is to call one of the static convenience
  functions, e.g. getFont().

  Examples:

  \code
    bool ok;
    QFont font = QFontDialog::getFont(
                    &ok, QFont("Helvetica [Cronyx]", 10), this);
    if (ok) {
        // the user clicked OK and font is set to the font the user selected
    } else {
        // the user canceled the dialog; font is set to the initial
        // value, in this case Helvetica [Cronyx], 10
    }
  \endcode

    The dialog can also be used to set a widget's font directly:
  \code
    myWidget.setFont(QFontDialog::getFont(0, myWidget.font()));
  \endcode
  If the user clicks OK the font they chose will be used for myWidget,
  and if they click Cancel the original font is used.

  \image plastique-fontdialog.png A font dialog in the Plastique widget style.

  \sa QFont, QFontInfo, QFontMetrics
*/


/*!
  \internal
  Constructs a standard font dialog.

  Use setFont() to set the initial font attributes.

  The \a parent, \a name, \a modal and \a f parameters are passed to
  the QDialog constructor.

  \sa getFont()
*/

SmallFontDialog::SmallFontDialog(QWidget *parent, bool modal, Qt::WFlags f)
    : QDialog(parent, f)
{
    setModal(modal);
    setSizeGripEnabled(true);
    // grid
    familyEdit = new QLineEdit(this);
    familyEdit->setReadOnly(true);
    familyList = new QFontListView(this);
    familyEdit->setFocusProxy(familyList);

    familyAccel = new QLabel(tr("&Font"), this);
#ifndef QT_NO_SHORTCUT
    familyAccel->setBuddy(familyList);
#endif
    familyAccel->setIndent(2);

    sizeEdit = new QLineEdit(this);
    sizeEdit->setFocusPolicy(Qt::ClickFocus);
    QIntValidator *validator = new QIntValidator(1, 512, this);
    sizeEdit->setValidator(validator);
    sizeList = new QFontListView(this);

    sizeAccel = new QLabel(tr("&Size"), this);
#ifndef QT_NO_SHORTCUT
    sizeAccel->setBuddy(sizeEdit);
#endif
    sizeAccel->setIndent(2);

    sample = new QGroupBox(tr("Sample"), this);
    QHBoxLayout *hbox = new QHBoxLayout(sample);
    sampleEdit = new QLineEdit(sample);
    sampleEdit->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    sampleEdit->setAlignment(Qt::AlignCenter);
    // Note that the sample text is *not* translated with tr(), as the
    // characters used depend on the charset encoding.
    sampleEdit->setText("AaBbYyZz");
    hbox->addWidget(sampleEdit);

    size = 0;
    smoothScalable = false;

    connect(familyList, SIGNAL(highlighted(int)),
            SLOT(familyHighlighted(int)));
    connect(sizeEdit, SIGNAL(textChanged(QString)),
            SLOT(sizeChanged(QString)));
    connect(sizeList, SIGNAL(highlighted(int)),
            SLOT(sizeHighlighted(int)));

    updateFamilies();
    if (familyList->count() != 0)
        familyList->setCurrentItem(0);

    // grid layout
    QGridLayout * mainGrid = new QGridLayout(this);
    int margin = mainGrid->margin();
    int spacing = mainGrid->spacing();
    mainGrid->setSpacing(0);

    mainGrid->addWidget(familyAccel, 0, 0);
    mainGrid->addWidget(familyEdit, 1, 0);
    mainGrid->addWidget(familyList, 2, 0);

    mainGrid->addWidget(sizeAccel, 0, 2);
    mainGrid->addWidget(sizeEdit, 1, 2);
    mainGrid->addWidget(sizeList, 2, 2);

    mainGrid->setColumnStretch(0, 38);
    mainGrid->setColumnStretch(2, 10);

    mainGrid->setColumnMinimumWidth(1, spacing);

    mainGrid->addWidget(sample, 4, 0, 1, 2);

    mainGrid->setRowMinimumHeight(4, 8*margin);

    mainGrid->setRowMinimumHeight(5, margin);

    QHBoxLayout *buttonBox = new QHBoxLayout;
    mainGrid->addLayout(buttonBox, 6, 0, 1, 4);

    buttonBox->addStretch(1);
    QString okt = modal ? tr("OK") : tr("Apply");
    ok = new QPushButton(okt, this);
    if (modal)
        connect(ok, SIGNAL(clicked()), SLOT(accept()));
    ok->setDefault(true);

    QString cancelt = modal ? tr("Abbrechen") : tr("Schließen");
    cancel = new QPushButton(cancelt, this);
    connect(cancel, SIGNAL(clicked()), SLOT(reject()));

    resize(500, 360);

    sizeEdit->installEventFilter(this);
    familyList->installEventFilter(this);
    sizeList->installEventFilter(this);

    familyList->setFocus();
    buttonBox->addWidget(ok);
    buttonBox->addSpacing(spacing);
    buttonBox->addWidget(cancel);
}

/*!
  \internal
 Destroys the font dialog and frees up its storage.
*/

SmallFontDialog::~SmallFontDialog()
{
}

/*!
  Executes a modal font dialog and returns a font.

  If the user clicks OK, the selected font is returned. If the user
  clicks Cancel, the \a initial font is returned.

  The dialog is constructed with the given \a parent.
  \a initial is the initially selected font.
  If the \a ok parameter is not-null, \e *\a ok is set to true if the
  user clicked OK, and set to false if the user clicked Cancel.

  This static function is less flexible than the full QFontDialog
  object, but is convenient and easy to use.

  Examples:
  \code
    bool ok;
    QFont font = QFontDialog::getFont(&ok, QFont("Times", 12), this);
    if (ok) {
        // font is set to the font the user selected
    } else {
        // the user canceled the dialog; font is set to the initial
        // value, in this case Times, 12.
    }
  \endcode

    The dialog can also be used to set a widget's font directly:
  \code
    myWidget.setFont(QFontDialog::getFont(0, myWidget.font()));
  \endcode
  In this example, if the user clicks OK the font they chose will be
  used, and if they click Cancel the original font is used.
*/
QFont SmallFontDialog::getFont(bool *ok, const QFont &initial,
                            QWidget *parent)
{
    return getFont(ok, &initial, parent);
}

/*!
    \overload

  Executes a modal font dialog and returns a font.

  If the user clicks OK, the selected font is returned. If the user
  clicks Cancel, the Qt default font is returned.

  The dialog is constructed with the given \a parent.
  If the \a ok parameter is not-null, \e *\a ok is set to true if the
  user clicked OK, and false if the user clicked Cancel.

  This static function is less functional than the full QFontDialog
  object, but is convenient and easy to use.

  Example:
  \code
    bool ok;
    QFont font = QFontDialog::getFont(&ok, this);
    if (ok) {
        // font is set to the font the user selected
    } else {
        // the user canceled the dialog; font is set to the default
        // application font, QApplication::font()
    }
  \endcode

*/
QFont SmallFontDialog::getFont(bool *ok, QWidget *parent)
{
    return getFont(ok, 0, parent);
}

QFont SmallFontDialog::getFont(bool *ok, const QFont *def, QWidget *parent)
{
    QFont result;
    if (def)
        result = *def;

    SmallFontDialog *dlg = new SmallFontDialog(parent, true);

    dlg->setFont((def ? *def : QFont()));
    dlg->setWindowTitle(tr("Select Font"));

    bool res = (dlg->exec() == QDialog::Accepted);
    if (res)
        result = dlg->font();
    if (ok)
        *ok = res;
    delete dlg;
    return result;
}


/*!
    \internal
    An event filter to make the Up, Down, PageUp and PageDown keys work
    correctly in the line edits. The source of the event is the object
    \a o and the event is \a e.
*/

bool SmallFontDialog::eventFilter(QObject * o , QEvent * e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent * k = (QKeyEvent *)e;
        if (o == sizeEdit &&
        (k->key() == Qt::Key_Up ||
             k->key() == Qt::Key_Down ||
         k->key() == Qt::Key_PageUp ||
         k->key() == Qt::Key_PageDown)) {

            int ci = sizeList->currentItem();
            (void)QApplication::sendEvent(sizeList, k);

            if (ci != sizeList->currentItem() && false)
                sizeEdit->selectAll();
            return true;
        } else if ((o == familyList) &&
                    (k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter)) {
            k->accept();
        accept();
            return true;
        }
    } else if (e->type() == QEvent::FocusIn && false) {
        if (o == familyList)
            familyEdit->selectAll();
        else if (o == sizeList)
            sizeEdit->selectAll();
    } else if (e->type() == QEvent::MouseButtonPress && o == sizeList) {
            sizeEdit->setFocus();
    }
    return QDialog::eventFilter(o, e);
}

void SmallFontDialog::parseFontName(const QString &name, QString &foundry, QString &family)
{
    int i = name.indexOf('[');
    int li = name.lastIndexOf(']');
    if (i >= 0 && li >= 0 && i < li) {
        foundry = name.mid(i + 1, li - i - 1);
        if (name[i - 1] == ' ')
            i--;
        family = name.left(i);
    } else {
        foundry.clear();
        family = name;
    }

    // capitalize the family/foundry names
    bool space = true;
    QChar *s = family.data();
    int len = family.length();
    while(len--) {
        if (space) *s = s->toUpper();
        space = s->isSpace();
        ++s;
    }

    space = true;
    s = foundry.data();
    len = foundry.length();
    while(len--) {
        if (space) *s = s->toUpper();
        space = s->isSpace();
        ++s;
    }
}

/*!
  \internal
    Updates the contents of the "font family" list box. This
  function can be reimplemented if you have special requirements.
*/

void SmallFontDialog::updateFamilies()
{
   familyList->blockSignals(true);

    enum match_t { MATCH_NONE=0, MATCH_LAST_RESORT=1, MATCH_APP=2, MATCH_FALLBACK, MATCH_FAMILY=3 };

    QStringList familyNames = fdb.families();

    familyNames.sort();

    familyList->model()->setStringList(familyNames);

    QString foundryName1, familyName1, foundryName2, familyName2;
    int bestFamilyMatch = -1;
    match_t bestFamilyType = MATCH_NONE;

    QFont f;

    // ##### do the right thing for a list of family names in the font.
    parseFontName(family, foundryName1, familyName1);

    QStringList::Iterator it = familyNames.begin();
    int i = 0;
    for(; it != familyNames.end(); ++it, ++i) {

        parseFontName(*it, foundryName2, familyName2);

        //try to match..
        if (familyName1 == familyName2) {
            bestFamilyType = MATCH_FAMILY;
            if (foundryName1 == foundryName2) {
                bestFamilyMatch = i;
                break;
            }
            if (bestFamilyMatch < MATCH_FAMILY)
                bestFamilyMatch = i;
        }

        //and try some fall backs
        match_t type = MATCH_NONE;
        if (bestFamilyType <= MATCH_NONE && familyName2 == f.lastResortFamily())
            type = MATCH_LAST_RESORT;
        if (bestFamilyType <= MATCH_LAST_RESORT && familyName2 == f.family())
            type = MATCH_APP;
        // ### add fallback for writingSystem
        if (type != MATCH_NONE) {
            bestFamilyType = type;
            bestFamilyMatch = i;
        }
    }

    if (i != -1 && bestFamilyType != MATCH_NONE)
        familyList->setCurrentItem(bestFamilyMatch);
    else
        familyList->setCurrentItem(0);
    familyEdit->setText(familyList->currentText());
    if (false &&
        (familyList->hasFocus()))
        familyEdit->selectAll();

    familyList->blockSignals(false);
    updateSizes();
}

/*!
    \internal
    Updates the contents of the "font size" list box. This
  function can be reimplemented if you have special requirements.
*/

void SmallFontDialog::updateSizes()
{
    sizeList->blockSignals(true);

    if (!familyList->currentText().isEmpty()) {
        QList<int> sizes = fdb.pointSizes(familyList->currentText());

        int i = 0;
        int current = -1;
        QStringList str_sizes;
        for(QList<int>::const_iterator it = sizes.constBegin() ; it != sizes.constEnd(); ++it) {
            str_sizes.append(QString::number(*it));
            if (current == -1 && *it >= size)
                current = i;
            ++i;
        }
        sizeList->model()->setStringList(str_sizes);
        if (current == -1)
            // we request a size bigger than the ones in the list, select the biggest one
            current = sizeList->count() - 1;
        sizeList->setCurrentItem(current);

        sizeEdit->blockSignals(true);
        sizeEdit->setText((smoothScalable ? QString::number(size) : sizeList->currentText()));
        if (false &&
             (sizeList->hasFocus()))
            sizeEdit->selectAll();
        sizeEdit->blockSignals(false);
    } else {
        sizeEdit->clear();
    }

    sizeList->blockSignals(false);
    updateSample();
}

void SmallFontDialog::updateSample()
{
    if (familyList->currentText().isEmpty())
        sampleEdit->clear();
    else {
        sampleEdit->setFont(font());
        sampleEdit->setText("AaBbYyZz");
    }
}

/*!
    \internal
*/
void SmallFontDialog::familyHighlighted(int i)
{
    family = familyList->text(i);
    familyEdit->setText(family);
    if (false &&
         familyList->hasFocus())
        familyEdit->selectAll();
    updateSample();
}


/*!
    \internal
*/

void SmallFontDialog::sizeHighlighted(int index)
{
    QString s = sizeList->text(index);
    sizeEdit->setText(s);
    if (false &&
         sizeEdit->hasFocus())
        sizeEdit->selectAll();

    size = s.toInt();
    updateSample();
}

/*!
    \internal
    This slot is called if the user changes the font size.
    The size is passed in the \a s argument as a \e string.
*/

void SmallFontDialog::sizeChanged(const QString &s)
{
    // no need to check if the conversion is valid, since we have an QIntValidator in the size edit
    int size = s.toInt();
    if (this->size == size)
        return;

    this->size = size;
    if (sizeList->count() != 0) {
        int i;
        for (i = 0 ; i < sizeList->count() - 1 ; i++) {
            if (sizeList->text(i).toInt() >= this->size)
                break;
        }
        sizeList->blockSignals(true);
        sizeList->setCurrentItem(i);
        sizeList->blockSignals(false);
    }
    updateSample();
}

/*!
  \internal
  Sets the font highlighted in the QFontDialog to font \a f.

  \sa font()
*/

void SmallFontDialog::setFont(const QFont &f)
{
    family = f.family();
    size = f.pointSize();
    if (size == -1) {
            QFontInfo fi(f);
            size = fi.pointSize();
    }
    updateFamilies();
}

/*!
  \internal
  Returns the font which the user has chosen.

  \sa setFont()
*/

QFont SmallFontDialog::font() const
{
    int pSize = sizeEdit->text().toInt();

    QFont f = fdb.font(familyList->currentText(), style, pSize);
    return f;
}

/*!
    \fn QFont QFontDialog::getFont(bool *ok, const QFont &def, QWidget* parent, const char* name)

    Call getFont(\a ok, \a def, \a parent) instead.

    The \a name parameter is ignored.
*/

/*!
    \fn QFont QFontDialog::getFont(bool *ok, QWidget* parent, const char* name)

    Call getFont(\a ok, \a parent) instead.

    The \a name parameter is ignored.
*/

#include "smallfontdialog.moc"
#include "moc_smallfontdialog.cpp"

