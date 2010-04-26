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
* Project:		QPgAdmin
* Author:		PanteR
* Contact:		panter.dsd@gmail.com
*******************************************************************/

#include <QtCore/QSettings>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <QtGui/QAction>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QDockWidget>
#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>

#include "mainwindow.h"
#include "databasetree.h"
#include "edittablewidget.h"
#include "sqlquerywidget.h"

MainWindow::MainWindow(QWidget* parent, Qt::WFlags f)
		: QMainWindow(parent, f)
{
	mdiArea = new QMdiArea (this);
	setCentralWidget(mdiArea);

	databaseTree = new DatabaseTree (this);
	connect (databaseTree, SIGNAL (openTable (QString, QString)), this, SLOT (openTable (QString, QString)));

	databaseTreeDock = new QDockWidget (this);
	databaseTreeDock->setObjectName("TREE_DOCK");
	databaseTreeDock->setAllowedAreas (Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	databaseTreeDock->setWidget(databaseTree);
	addDockWidget(Qt::LeftDockWidgetArea, databaseTreeDock);

	QMenuBar *mainMenu = new QMenuBar (this);
	setMenuBar (mainMenu);

	QMenu *instrumentsMenu = new QMenu (tr ("Instruments"), this);
	mainMenu->addMenu (instrumentsMenu);

	QMenu *viewMenu = new QMenu (tr ("View"), this);
	mainMenu->addMenu (viewMenu);

	actionSqlEdit = new QAction (this);
	connect (actionSqlEdit, SIGNAL (triggered ()), this, SLOT (sqlEdit ()));
	instrumentsMenu->addAction (actionSqlEdit);

	actionShowHideDatabaseTree = new QAction (this);
	actionShowHideDatabaseTree->setShortcut (Qt::CTRL + Qt::Key_E);
	actionShowHideDatabaseTree->setCheckable (true);
	actionShowHideDatabaseTree->setChecked (true);
	connect (actionShowHideDatabaseTree, SIGNAL (toggled (bool)), databaseTreeDock, SLOT (setShown (bool)));
	viewMenu->addAction (actionShowHideDatabaseTree);

	loadSettings();
	retranslateStrings();
}

MainWindow::~MainWindow()
{
	saveSettings();
}

void MainWindow::retranslateStrings()
{
	databaseTreeDock->setWindowTitle (databaseTree->windowTitle ());

	actionShowHideDatabaseTree->setText (tr ("Show database tree"));
}

void MainWindow::loadSettings()
{
	QSettings settings;

	if (settings.value("Global/RestoreWindowParams", true).toBool()) {
		settings.beginGroup("MainWindow");
		move(settings.value("pos", QPoint(0, 0)).toPoint());
		resize(settings.value("size", QSize(640, 480)).toSize());
		bool isMaximized = settings.value("IsMaximized", false).toBool();
		if (isMaximized)
			setWindowState(Qt::WindowMaximized);
		restoreState(settings.value("State", "").toByteArray());
		settings.endGroup();
	}
}

void MainWindow::saveSettings()
{
	QSettings settings;

	settings.beginGroup("MainWindow");
	if (windowState() != Qt::WindowMaximized) {
		settings.setValue("pos", pos());
		settings.setValue("size", size());
		settings.setValue("IsMaximized", false);
	} else
		settings.setValue("IsMaximized", true);
	settings.setValue("State", saveState());
	settings.endGroup();

	settings.sync();
}

bool MainWindow::event(QEvent *ev)
{
	if (ev->type() == QEvent::LanguageChange) {
		retranslateStrings();
	}

	if (ev->type () == QEvent::Close) {
		QList <QMdiSubWindow*> l = findChildren <QMdiSubWindow*> ();
		foreach (QMdiSubWindow *w, l) {
			if (!w->close ()) {
				ev->ignore ();
				return false;
			}
		}
	}

	return QMainWindow::event(ev);
}

void MainWindow::openTable (const QString& connectionName, const QString& tableName)
{
	QMdiSubWindow *mdi = new QMdiSubWindow (this);

	EditTableWidget *w = new EditTableWidget (connectionName, tableName, mdi);

	mdi->setWidget (w);
	mdi->setAttribute(Qt::WA_DeleteOnClose);
	mdiArea->addSubWindow(mdi);
	mdi->show ();
}

void MainWindow::sqlEdit ()
{
	QMdiSubWindow *mdi = new QMdiSubWindow (this);

	SqlQueryWidget *w = new SqlQueryWidget ("", mdi);
	connect (databaseTree, SIGNAL (connectionsChanged ()), w, SLOT (connectionsChanged ()));

	mdi->setWidget (w);
	mdi->setAttribute(Qt::WA_DeleteOnClose);
	mdiArea->addSubWindow(mdi);
	mdi->show ();
}
