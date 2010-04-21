#ifndef ConnectionDialog_H
#define ConnectionDialog_H

class QLabel;
class QLineEdit;
class QSpinBox;
class QCheckBox;

#include <QtGui/QDialog>

class ConnectionDialog : public QDialog {
	Q_OBJECT

private:
	QLabel *connectionNameLabel;
	QLineEdit *connectionNameEdit;

	QLabel *hostLabel;
	QLineEdit *hostEdit;

	QLabel *portLabel;
	QSpinBox *portEdit;

	QLabel *userNameLabel;
	QLineEdit *userNameEdit;

	QLabel *passwordLabel;
	QLineEdit *passwordEdit;

	QCheckBox *savePasswordBox;

public:
	ConnectionDialog(QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowSystemMenuHint);
	virtual ~ConnectionDialog()
	{}

	QString connectionName () const;
	void setConnectionName (const QString& value);

	QString host () const;
	void setHost (const QString& value);

	int port () const;
	void setPort (int value);

	QString userName () const;
	void setUserName (const QString& value);

	QString password () const;
	void setPassword (const QString& value);

	bool isSavePassword () const;
};
#endif
