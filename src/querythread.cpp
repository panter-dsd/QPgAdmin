#include <QtCore/QStringList>

#include <QtSql/QSqlDatabase>

#include "querythread.h"

QueryThread::QueryThread(const QString &connectionName, const QString &queryString, QObject *parent)
	: QThread(parent)
	, m_connectionName(connectionName)
	, m_queryString(queryString)
{

}

QueryThread::~QueryThread()
{

}

void QueryThread::run()
{
	executeQuery(m_queryString);
}

QSqlQuery QueryThread::lastQuery()
{
	return m_query;
}

bool QueryThread::executeQuery(const QString &queryString)
{
	QSqlQuery query(QSqlDatabase::database(m_connectionName));

	const int result = query.exec(queryString);
	m_query = query;

	return result;
}
