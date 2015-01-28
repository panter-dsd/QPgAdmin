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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class QAction;
class DatabaseTree;
class QDockWidget;
class QMdiArea;
class QMenu;

#include <QtGui/QMainWindow>

class MainWindow : public QMainWindow
{
	Q_OBJECT

private:
	QMdiArea *mdiArea;

	DatabaseTree *databaseTree;
	QDockWidget *databaseTreeDock;

	QMenu *instrumentsMenu;
	QMenu *viewMenu;

	QAction *actionSqlEdit;
	QAction *actionShowHideDatabaseTree;

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags f = 0);
	~MainWindow();

private:
	void loadSettings();
	void saveSettings();
	void retranslateStrings();

protected:
	bool event(QEvent *ev);

private Q_SLOTS:
	void openTable(const QString &connectionName, const QString &tableName);
	void sqlEdit();
};

#endif // DBFREDACTORMAINWINDOW_H
