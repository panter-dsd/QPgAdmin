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

#ifndef EDITTABLEWIDGET_H
#define EDITTABLEWIDGET_H

class QTableView;
class QSqlTableModel;
class QToolBar;
class QAction;

#include <QtCore/QModelIndex>

#include <QtGui/QWidget>

class EditTableWidget : public QWidget
{
	Q_OBJECT

private:
	QSqlTableModel *model;
	QTableView *view;
	QToolBar *toolBar;

	QAction *actionSave;
	QAction *actionRevert;
	QAction *actionAddIncludeFilter;
	QAction *actionAddExcludeFilter;

public:
	EditTableWidget(const QString &connectionName, const QString &tableName, QWidget *parent = 0);
	~EditTableWidget();

private:
	void retranslateStrings();
	QString dataForFilter(const QModelIndex &index);

protected:
	bool event(QEvent *ev);

private Q_SLOTS:
	void addIncludeFilter();
	void addExcludeFilter();
};

#endif //EDITTABLEWIDGET_H
