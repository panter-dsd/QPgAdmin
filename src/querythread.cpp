#include <QtCore/QStringList>

#include <QtSql/QSqlDatabase>

#include "querythread.h"

QueryThread::QueryThread (const QString& connectionName, const QString& queryString, QObject* parent)
	: QThread (parent), m_connectionName (connectionName), m_queryString (queryString)
{

}

void QueryThread::run ()
{
	const QStringList queryStrings = m_queryString.split (";");
	std::all_of(queryStrings.begin(), queryStrings.end(),
			[this] (const QString &queryString) {
				const QString query = queryString.simplified();
			    return query.isEmpty() || executeQuery (query.simplified());
			});
}

bool QueryThread::executeQuery (const QString &queryString)
{
	QSqlQuery query (QSqlDatabase::database (m_connectionName));

	const int result = query.exec (queryString);
	m_query = query;

	return result;
}
