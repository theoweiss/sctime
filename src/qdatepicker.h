/*  -*- C++ -*-
    This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998-2001 Mirko Boehm (mirko@kde.org)
              (C) 2003 Florian Schmitt <f.schmitt@science-computing.de>
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// This file has been ported from KDE to plain QT

#ifndef QDATEPICKER_H
#define QDATEPICKER_H
#include <qdatetime.h>
#include <q3frame.h>
#include <QValidator>
#include <QCalendarWidget>
//Added by qt3to4:
#include <QEvent>
#include <QResizeEvent>
#include <QLineEdit>
#include <Q3GridView>

class QToolButton;

/** Week selection widget.
* @internal
* @version $Id$
* @author Stephan Binner
*/
class QDateInternalWeekSelector : public QLineEdit
{
  Q_OBJECT
protected:
  QIntValidator *val;
  int result;
public slots:
  void weekEnteredSlot();
  void setMaxWeek(int max);
signals:
  void closeMe(int);
public:
  QDateInternalWeekSelector( QWidget* parent=0, const char* name=0);
  int getWeek();
  void setWeek(int week);

private:
  class QDateInternalWeekPrivate;
  QDateInternalWeekPrivate *d;
};

/**
* Validates user-entered dates.
*/
class QDateValidator : public QValidator
{
public:
    QDateValidator(QWidget* parent=0, const char* name=0);
    virtual State validate(QString&, int&) const;
    virtual void fixup ( QString & input ) const;
    State date(const QString&, QDate&) const;
};

/**
 * Frame with popup menu behavior.
 * @author Tim Gilman, Mirko Boehm
 * @version $Id$
 */
class QPopupFrame : public Q3Frame
{
  Q_OBJECT
protected:
  /**
   * The result. It is returned from exec() when the popup window closes.
   */
  int result;
  /**
   * Catch key press events.
   */
  virtual void keyPressEvent(QKeyEvent* e);
  /**
   * The only subwidget that uses the whole dialog window.
   */
  QWidget *main;
public slots:
  /**
   * Close the popup window. This is called from the main widget, usually.
   * @p r is the result returned from exec().
   */
  void close(int r);
public:
  /**
   * The contructor. Creates a dialog without buttons.
   */
  QPopupFrame(QWidget* parent=0, const char*  name=0);
  /**
   * Set the main widget. You cannot set the main widget from the constructor,
   * since it must be a child of the frame itselfes.
   * Be careful: the size is set to the main widgets size. It is up to you to
   * set the main widgets correct size before setting it as the main
   * widget.
   */
  void setMainWidget(QWidget* m);
  /**
   * The resize event. Simply resizes the main widget to the whole
   * widgets client size.
   */
  virtual void resizeEvent(QResizeEvent*);
  /**
   * Open the popup window at position pos.
   */
  void popup(const QPoint &pos);
  /**
   * Execute the popup window.
   */
  int exec(QPoint p);
  /**
   * Dito.
   */
  int exec(int x, int y);

private:

  virtual bool close(bool alsoDelete) { return Q3Frame::close(alsoDelete); }
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class QPopupFramePrivate;
  QPopupFramePrivate *d;
};

/**
 * Provides a widget for calendar date input.
 *
 *     Different from the
 *     previous versions, it now emits two types of signals, either
 * dateSelected() or dateEntered() (see documentation for both
 *     signals).
 *
 *     A line edit has been added in the newer versions to allow the user
 *     to select a date directly by entering numbers like 19990101
 *     or 990101.
 *
 * \image html kdatepicker.png "KDE Date Widget"
 *
 *     @version $Id$
 *     @author Tim Gilman, Mirko Boehm
 *
 * @short A date selection widget.
 **/
class QDatePicker: public Q3Frame
{
  Q_OBJECT
  Q_PROPERTY( QDate date READ date WRITE setDate)
  Q_PROPERTY( int fontSize READ fontSize WRITE setFontSize )

public:
  /** The usual constructor.  The given date will be displayed
   * initially.
   **/
  QDatePicker(QWidget *parent=0,
              QDate=QDate::currentDate(),
              const char *name=0);

  /** The usual constructor.  The given date will be displayed
   * initially.
   * @since 3.1
   **/
  QDatePicker(QWidget *parent,
              QDate,
              const char *name,
              Qt::WFlags f); // ### KDE 4.0: Merge

  /**
   * Standard qt widget constructor. The initial date will be the
   * current date.
   * @since 3.1
   */
  QDatePicker( QWidget *parent, const char *name );

  /**
   * The destructor.
   **/
  virtual ~QDatePicker();

  /** The size hint for date pickers. The size hint recommends the
   *   minimum size of the widget so that all elements may be placed
   *  without clipping. This sometimes looks ugly, so when using the
   *  size hint, try adding 28 to each of the reported numbers of
   *  pixels.
   **/
  QSize sizeHint() const;

  /**
   * Sets the date.
   *
   *  @returns @p false and does not change anything
   *      if the date given is invalid.
   **/
  bool setDate(const QDate&);

  /**
   * @returns the selected date.
   */
  const QDate date() const;

  /**
   * Enables or disables the widget.
   **/
  void setEnabled(bool);

  /**
   * @returns the KDateTable widget child of this KDatePicker
   * widget.
   * @since 3.2
   */
  QCalendarWidget *dateTable() const { return table; };

  /**
   * Sets the font size of the widgets elements.
   **/
  void setFontSize(int);
  /**
   * Returns the font size of the widget elements.
   */
  int fontSize() const
    { return fontsize; }

protected:
  /// to catch move keyEvents when QLineEdit has keyFocus
  virtual bool eventFilter(QObject *o, QEvent *e );
  /// the resize event
  virtual void resizeEvent(QResizeEvent*);
  /// the line edit to enter the date directly
  QLineEdit *line;
  /// the validator for the line edit:
  QDateValidator *val;
  /// the date table
  QCalendarWidget *table;
  /// the size calculated during resize events
    //  QSize sizehint;
  /// the widest month string in pixels:
  QSize maxMonthRect;
protected slots:
  void dateChangedSlot();
  void tableClickedSlot(QDate date);

  /**
   * @since 3.1
   */
  void lineEnterPressed();
  /**
   * @since 3.2
   */
  void todayButtonClicked();
  /**
   * @since 3.2
   */
  void weekSelected(int);

  void currentPageChangedSlot ( int year, int month );

signals:

  /** This signal is emitted each time the selected date is changed.
   *  Usually, this does not mean that the date has been entered,
   *  since the date also changes, for example, when another month is
   *  selected.
   *  @see dateSelected
   */
  void dateChanged(QDate);
  /** This signal is emitted each time a day has been selected by
   *  clicking on the table (hitting a day in the current month). It
   *  has the same meaning as dateSelected() in older versions of
   *  KDatePicker.
   */
  void dateSelected(QDate);
  /** This signal is emitted when enter is pressed and a VALID date
   *  has been entered before into the line edit. Connect to both
   *  dateEntered() and dateSelected() to receive all events where the
   *  user really enters a date.
   */
  void dateEntered(QDate);
  /** This signal is emitted when the day has been selected by
   *  clicking on it in the table.
   */
  void tableClicked();

private:
  /// the font size for the widget
  int fontsize;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  void init( const QDate &dt );
  void fillWeeksCombo(const QDate &date);
  class QDatePickerPrivate;
  QDatePickerPrivate *d;
};

#endif //  QDATEPICKER_H
