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

#ifndef QUERYTHREAD_H
#define QUERYTHREAD_H

#include <QtCore/QThread>

#include <QtSql/QSqlQuery>

class QueryThread : public QThread
{
	Q_OBJECT

public:
	QueryThread(const QString &connectionName, const QString &queryString, QObject *parent = 0);
	virtual ~QueryThread();

	QSqlQuery lastQuery();

protected:
	virtual void run();

private:
	Q_DISABLE_COPY(QueryThread)

	bool executeQuery(const QString &queryString);

private:
	QString m_connectionName;
	QString m_queryString;

	QSqlQuery m_query;
};

#endif //QUERYTHREAD_H
