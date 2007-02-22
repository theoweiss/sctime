#ifndef COLORCHOOSER_H
#define COLORCHOOSER_H

#include "ui_colorchooserbase.h"
#include <QDialog>

class ColorChooser: public QDialog, private Ui::ColorChooserBase
{
  Q_OBJECT;
  public:
    ColorChooser(bool enableOwnColor, const QColor& color, QWidget* parent=NULL);
    virtual ~ColorChooser() {};
    QColor* selectedColor();
  private slots:
    void colorButtonPressed(QAbstractButton * button);
    void activateOwnColor(bool activate);
  private:
    QButtonGroup m_colorButtonGroup;
    QColor m_selectedColor;
};

#endif
