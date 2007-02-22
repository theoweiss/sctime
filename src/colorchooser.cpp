#include "colorchooser.h"


ColorChooser::ColorChooser(bool enableOwnColor, const QColor& color, QWidget* parent): QDialog(parent)
{
  setupUi(this);
  useOwnColorCheckBox->setChecked(enableOwnColor);
  activateOwnColor(enableOwnColor);
  m_colorButtonGroup.addButton(colorButton1);
  m_colorButtonGroup.addButton(colorButton2);
  m_colorButtonGroup.addButton(colorButton3);
  m_colorButtonGroup.addButton(colorButton4);
  m_colorButtonGroup.addButton(colorButton5);
  m_colorButtonGroup.addButton(colorButton6);
  m_colorButtonGroup.addButton(colorButton7);
  m_colorButtonGroup.addButton(colorButton8);
  m_colorButtonGroup.addButton(colorButton9);
  m_colorButtonGroup.setExclusive(true);
  connect(&m_colorButtonGroup, SIGNAL(buttonPressed(QAbstractButton*)), this, SLOT(colorButtonPressed(QAbstractButton*)));
  connect(useOwnColorCheckBox, SIGNAL(toggled(bool)), this, SLOT(activateOwnColor(bool)));
  m_selectedColor=color;
  QPalette pal=chosenColorButton->palette();
  pal.setColor(QPalette::Button,color);
  chosenColorButton->setPalette(pal);
}

void ColorChooser::colorButtonPressed(QAbstractButton * button)
{
  m_selectedColor=button->palette().button().color();
  chosenColorButton->setPalette(button->palette());
  activateOwnColor(true);
}

void ColorChooser::activateOwnColor(bool activate)
{
  //colorGroupBox->setEnabled(activate);
  useOwnColorCheckBox->setChecked(activate);
  chosenGroupBox->setEnabled(activate);
  if (activate)
    chosenColorButton->setText("");
  else
    chosenColorButton->setText("keine");
}

QColor* ColorChooser::selectedColor()
{
  if (useOwnColorCheckBox->isChecked())
    return &m_selectedColor;
  else
    return NULL;
}
