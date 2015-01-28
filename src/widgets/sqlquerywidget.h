/********************************************************************
* Copyright (C) PanteR
*-------------------------------------------------------------------
*
* QPgAdmin is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* QPgAdmin is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Panther Commander; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor,
* Boston, MA 02110-1301 USA
*-------------------------------------------------------------------
* Project:      QPgAdmin
* Author:       PanteR
* Contact:      panter.dsd@gmail.com
*******************************************************************/

#ifndef SQLQUERYWIDGET_H
#define SQLQUERYWIDGET_H

class QTabWidget;
class QPlainTextEdit;
class QTableView;
class QToolBar;
class QAction;
class QSplitter;
class QComboBox;
class QSqlQueryModel;
class QStatusBar;

#include <QtCore/QTime>

#include <QtGui/QWidget>

class SqlQueryWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SqlQueryWidget(const QString &connectionName = QString::null, QWidget *parent = nullptr);
	virtual ~SqlQueryWidget();

private:
	void loadSettings();
	void saveSettings();
	void retranslateStrings();

	static QStringList removeComments(const QStringList &sqlQueryes);
	static QStringList removeBlankLines(const QStringList &sqlQueryes);

protected:

	bool event(QEvent *ev);
private Q_SLOTS:
	void updateTabCaptions();
	QPlainTextEdit *addSqlEditor();
	void open();
	bool save();
	bool saveAs();
	void start();
	void queryFinished();
	void undo();

	void redo();

public Q_SLOTS:
	void connectionsChanged();
	void updateActions();

	bool closeTab(int index);


private:
	QString connectionName_;
	QTime time_;
	int timer_;

	QTabWidget *inputTabs_;
	QTabWidget *outputTabs_;
	QList<QPlainTextEdit *> sqlEdits_;
	QPlainTextEdit *messagesEdit_;
	QTableView *outputTable_;
	QSqlQueryModel *outputModel_;
	QToolBar *toolBar_;
	QSplitter *splitter_;
	QComboBox *connectionEdit_;
	QStatusBar *statusBar_;

	QAction *actionAddSqlEditor_;
	QAction *actionOpen_;
	QAction *actionSave_;
	QAction *actionSaveAs_;
	QAction *actionStart_;
	QAction *actionStop_;
	QAction *actionUndo_;
	QAction *actionRedo_;
};

#endif //SQLQUERYWIDGET_H
