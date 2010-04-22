#include <QtCore/QStringList>

#include <QtSql/QSqlDatabase>

#include "querythread.h"

QueryThread::QueryThread (const QString& connectionName, const QString& queryString, QObject* parent)
	: QThread (parent), m_connectionName (connectionName), m_queryString (queryString)
{

}

void QueryThread::run ()
{
	const QStringList& queryStrings = m_queryString.split (";");

	QSqlQuery query (QSqlDatabase::database (m_connectionName));

	foreach (const QString& queryString, queryStrings) {
		const int result = query.exec (queryString.simplified ());
		m_query = query; 
		if (!result) {
			break;
		}
	}
}
