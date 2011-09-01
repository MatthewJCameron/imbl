#include "error.h"
#include "componentGui.h"
#include <QVBoxLayout>
#include <QLabel>


ComponentGui::ComponentGui(Component * _comp, bool _deleteComponent, QWidget * parent) :
  QWidget(parent),
  deleteComponent(_deleteComponent),
  comp(_comp)
{
  if (!comp)
    throw_error("No base component for component Gui. Report to Developer.");
  setObjectName("UI for the " + _comp->objectName());
  connect(comp, SIGNAL(connectionChanged(bool)),
          SLOT(updateConnection(bool)));
}




ComponentGui::~ComponentGui() {
  if (deleteComponent)
    delete comp;
}

Component * ComponentGui::component() const {
  return comp;
}




ComponentWidget::ComponentWidget(QWidget * wdg, QWidget * parent) :
  ComponentGui(new Component(), true, parent),
  widget(wdg)
{
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(widget);
  setLayout(layout);
}

ComponentWidget * ComponentWidget::duplicate(QWidget * parent) {
  QPushButton * retBut = new QPushButton("The widget is now living somewhere.\n"
                                         "Press me to bring it back", this);
  QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(retBut->sizePolicy().hasHeightForWidth());
  retBut->setSizePolicy(sizePolicy);

  connect(retBut, SIGNAL(clicked()), SLOT(bringBackWidget()));

  layout()->removeWidget(widget);
  layout()->addWidget(retBut);

  return new ComponentWidget(widget,parent);

}

void ComponentWidget::bringBackWidget() {
  QWidget * curwdg = layout()->takeAt(0)->widget();
  if (curwdg != widget)
    delete curwdg;
  layout()->addWidget(widget);
  emit returned();
}


const QString PsswDial::psswd = "asdf";

PsswDial::PsswDial(QWidget *parent)
  : QDialog(parent) {

  if (objectName().isEmpty())
    setObjectName("Password dialog");
  setModal(true);

  QVBoxLayout * verticalLayout = new QVBoxLayout(this);

  QLabel * label = new QLabel(this);
  label->setText("This functionality is potentially dangerous."
                 " Please confirm the action entering correct password"
                 " or contact beamline staff if you do not know it.");
  label->setWordWrap(true);
  verticalLayout->addWidget(label);

  passwd_line = new QLineEdit(this);
  passwd_line->setObjectName("passwd_line");
  passwd_line->setEchoMode(QLineEdit::Password);
  verticalLayout->addWidget(passwd_line);

  buttonBox = new QDialogButtonBox(this);
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  verticalLayout->addWidget(buttonBox);

}


bool PsswDial::ask(QWidget *parent) {

  PsswDial dial(parent);
  DialogCode ret = (DialogCode) dial.exec();

  if ( ret == QDialog::Rejected ) {
    return false;
  } else if ( dial.passwd_line->text() != psswd ) {
    inform("I am sorry, you have entered a wrong password.", parent);
    return false;
  } else {
    return true;
  }

}

