#include <QtGui/QLabel>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>
#include <QtGui/QCheckBox>
#include <QtGui/QLayout>

#include "connectiondialog.h"

ConnectionDialog::ConnectionDialog(QWidget * parent,Qt::WindowFlags f)
	: QDialog(parent, f)
{
	connectionNameLabel = new QLabel (tr ("Connection name"), this);

	connectionNameEdit = new QLineEdit (this);

	hostLabel = new QLabel (tr ("Host"), this);

	hostEdit = new QLineEdit (this);

	portLabel = new QLabel (tr ("Port"), this);

	portEdit = new QSpinBox (this);
	portEdit->setRange(0, 9999);
	portEdit->setValue(5432);

	userNameLabel = new QLabel (tr ("User name"), this);

	userNameEdit = new QLineEdit (this);

	passwordLabel = new QLabel (tr ("Password"), this);

	passwordEdit = new QLineEdit (this);
	passwordEdit->setEchoMode(QLineEdit::Password);

	savePasswordBox = new QCheckBox (tr ("Save password"), this);
	savePasswordBox->setChecked(true);

	QGridLayout *gridLayout = new QGridLayout ();
	gridLayout->addWidget(connectionNameLabel, 0, 0);
	gridLayout->addWidget(connectionNameEdit, 0, 1);
	gridLayout->addWidget(hostLabel, 1, 0);
	gridLayout->addWidget(hostEdit, 1, 1);
	gridLayout->addWidget(portLabel, 2, 0);
	gridLayout->addWidget(portEdit, 2, 1);
	gridLayout->addWidget(userNameLabel, 3, 0);
	gridLayout->addWidget(userNameEdit, 3, 1);
	gridLayout->addWidget(passwordLabel, 4, 0);
	gridLayout->addWidget(passwordEdit, 4, 1);
	gridLayout->addWidget(savePasswordBox, 5, 1);

	QDialogButtonBox *buttons = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
													  Qt::Horizontal,
													  this);
	connect (buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect (buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout *mainLayout = new QVBoxLayout ();
	mainLayout->addLayout(gridLayout);
	mainLayout->addWidget(buttons);
	setLayout(mainLayout);
}

QString ConnectionDialog::connectionName () const
{
	return connectionNameEdit->text();
}

void ConnectionDialog::setConnectionName (const QString& value)
{
	connectionNameEdit->setText(value);
}

QString ConnectionDialog::host () const
{
	return hostEdit->text();
}

void ConnectionDialog::setHost (const QString& value)
{
	hostEdit->setText(value);
}

int ConnectionDialog::port () const
{
	return portEdit->value();
}

void ConnectionDialog::setPort (int value)
{
	portEdit->setValue(value);
}

QString ConnectionDialog::userName () const
{
	return userNameEdit->text();
}

void ConnectionDialog::setUserName (const QString& value)
{
	userNameEdit->setText(value);
}

QString ConnectionDialog::password () const
{
	return passwordEdit->text();
}

void ConnectionDialog::setPassword (const QString& value)
{
	passwordEdit->setText(value);
}

bool ConnectionDialog::isSavePassword () const
{
	return savePasswordBox->isChecked();
}
