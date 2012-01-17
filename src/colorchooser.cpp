#include "colorchooser.h"
#if (defined Q_OS_MAC || defined WIN32)
#include <QStyleFactory>
#endif


ColorChooser::ColorChooser(bool enableOwnColor, const QColor& color, QWidget* parent): QDialog(parent)
{
  setupUi(this);
  useOwnColorCheckBox->setChecked(enableOwnColor);
  activateOwnColor(enableOwnColor);

#if (defined Q_OS_MAC || defined WIN32)
  /* Actual native Mac OS X Buttons can not have a background color. So switch
   * the color chooser buttons to motif style as a workaround. The proper fix
   * would be to change the dialog to use some other widget that supports
   * backgrounds on Mac OS X as well, like a ListView. */
  QStyle *buttonStyle = QStyleFactory::create("Motif");
  chosenColorButton->setStyle(buttonStyle);
  colorButton1->setStyle(buttonStyle);
  colorButton2->setStyle(buttonStyle);
  colorButton3->setStyle(buttonStyle);
  colorButton4->setStyle(buttonStyle);
  colorButton5->setStyle(buttonStyle);
  colorButton6->setStyle(buttonStyle);
  colorButton7->setStyle(buttonStyle);
  colorButton8->setStyle(buttonStyle);
  colorButton9->setStyle(buttonStyle);
#endif

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
