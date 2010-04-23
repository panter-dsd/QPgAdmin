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

#ifndef DATABASETREE_H
#define DATABASETREE_H

class QTreeWidget;
class QTreeWidgetItem;
class QAction;

#include <QtGui/QWidget>

class DatabaseTree : public QWidget {
	Q_OBJECT

private:
	struct Connection {
		QString name;
		QString host;
		int port;
		QString maintenanceBase;
		QString userName;
		QString password;
	};

private:
	QTreeWidget *tree;

	QAction *actionAddConnection;

	QList<Connection> connections;
public:
	DatabaseTree (QWidget *parent);
	~DatabaseTree ();

private:
	void loadSettings();
	void saveSettings();
	void retranslateStrings();
	void loadTree ();
	void loadDatabases (QTreeWidgetItem *parent);
	void loadSchemes (QTreeWidgetItem *parent);
	void loadTables (QTreeWidgetItem *parent);
	void loadViews (QTreeWidgetItem *parent);
	void loadSequences (QTreeWidgetItem *parent);

private Q_SLOTS:
	void addConnection ();
	void treeContextMenu (const QPoint& point);

	void itemExpanded (QTreeWidgetItem *item);
	void itemActivated (QTreeWidgetItem *item, int column);

Q_SIGNALS:
	void openTable (const QString& connectionName, const QString& tableName);
};

#endif //DATABASETREE_H
