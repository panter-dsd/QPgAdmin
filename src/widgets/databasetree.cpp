#include <QtCore/QSettings>
#include <QtCore/QDebug>

#include <QtGui/QTreeWidget>
#include <QtGui/QLayout>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QHeaderView>
#include <QtGui/QMessageBox>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include "databasetree.h"
#include "connectiondialog.h"

DatabaseTree::DatabaseTree (QWidget *parent)
	: QWidget (parent)
{
	setObjectName ("DATABASE_TREE");

	tree = new QTreeWidget (this);
	tree->header()->hide();
	tree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect (tree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(treeContextMenu(QPoint)));
	connect (tree, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(itemExpanded(QTreeWidgetItem*)));
	connect (tree, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(itemActivated(QTreeWidgetItem*,int)));

	QVBoxLayout *mainLayout = new QVBoxLayout ();
	mainLayout->setContentsMargins (0, 0, 0, 0);
	mainLayout->addWidget(tree);
	setLayout(mainLayout);

	actionAddConnection = new QAction (this);
	connect (actionAddConnection, SIGNAL(triggered()), this, SLOT(addConnection()));

	retranslateStrings ();
	loadSettings();
	loadTree ();
}

DatabaseTree::~DatabaseTree()
{
	saveSettings();
}

void DatabaseTree::loadSettings()
{
	QSettings settings;

	int size = settings.beginReadArray("Connections");

	for (int i = 0; i < size; i++) {
		settings.setArrayIndex(i);

		Connection c;

		c.name = settings.value("Name").toString();
		c.host = settings.value("Host").toString();
		c.port = settings.value("Port").toInt();
		c.userName = settings.value("UserName").toString();
		c.password = settings.value("Password").toString();

		connections.append(c);
	}
	settings.endArray();
}

void DatabaseTree::saveSettings()
{
	QSettings settings;

	settings.beginWriteArray("Connections", connections.size());

	for (int i = 0, size = connections.size(); i < size; i++) {
		settings.setArrayIndex(i);

		settings.setValue("Name", connections.at(i).name);
		settings.setValue("Host", connections.at(i).host);
		settings.setValue("Port", connections.at(i).port);
		settings.setValue("UserName", connections.at(i).userName);
		settings.setValue("Password", connections.at(i).password);
	}
	settings.endArray();
}

void DatabaseTree::retranslateStrings()
{
	setWindowTitle (tr ("Database tree"));
	actionAddConnection->setText(tr ("Add connection"));
}

void DatabaseTree::addConnection ()
{
	ConnectionDialog d (this);
	if (d.exec()) {
		Connection c;
		c.name = d.connectionName();
		c.host = d.host();
		c.port = d.port();
		c.userName = d.userName();
		c.password = d.password();
		connections.append(c);

		loadTree();
	}
}

void DatabaseTree::treeContextMenu (const QPoint& point)
{
	QMenu menu;

	menu.addAction(actionAddConnection);

	QTreeWidgetItem *item = tree->itemAt(point);

	menu.exec(tree->mapToGlobal(point));
}

void DatabaseTree::loadTree ()
{

	QTreeWidgetItem *item;

	for (int i = 0, size = connections.size(); i < size; i++) {

		item = 0;
		const QList<QTreeWidgetItem*> l = tree->findItems(connections.at(i).name, Qt::MatchFixedString, 0);
		foreach (QTreeWidgetItem *i, l) {
			if (i->parent() == 0) {
				item = i;
				break;
			}
		}

		if (!item) {
			item = new QTreeWidgetItem ();
			item->setText(0, connections.at(i).name);

			item->setData(0, Qt::UserRole, "CONNECTION");
			item->setData(1, Qt::UserRole, i);
			item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

			tree->addTopLevelItem(item);
		}

		if (QSqlDatabase::database(item->text(0)).isOpen()) {
			loadDatabases (item);
			item->setData(0, Qt::DecorationRole, QIcon(":/share/images/connect_established.png"));
		} else {
			item->setData(0, Qt::DecorationRole, QIcon(":/share/images/connect_no.png"));
		}
	}
}

void DatabaseTree::loadDatabases (QTreeWidgetItem *parent)
{
	QSqlQuery query (QSqlDatabase::database(parent->text(0)));
	if (!query.exec("SELECT datname FROM pg_database")) {
		QMessageBox::critical(this, "", query.lastError().text());
		return;
	}

	QTreeWidgetItem *item = 0;
	const QList<QTreeWidgetItem*> l = tree->findItems(tr ("Databases"), Qt::MatchFixedString | Qt::MatchRecursive, 0);
	foreach (QTreeWidgetItem *i, l) {
		if (i->parent() == parent) {
			item = i;
			break;
		}
	}
	if (!item) {
		item = new QTreeWidgetItem ();
		item->setText(0, tr ("Databases"));
		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

		parent->addChild(item);
	}
	parent = item;

	while (query.next()) {
		item = 0;
		const QList<QTreeWidgetItem*> l = tree->findItems(query.value(0).toString(), Qt::MatchFixedString | Qt::MatchRecursive, 0);
		foreach (QTreeWidgetItem *i, l) {
			if (i->parent() == parent) {
				item = i;
				break;
			}
		}

		if (!item) {
			item = new QTreeWidgetItem ();
			item->setText(0, query.value(0).toString());
			item->setData(0, Qt::UserRole, "DATABASE");
			item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

			parent->addChild(item);
		}
		if (QSqlDatabase::database(parent->parent ()->text(0) + "." + item->text(0)).isOpen()) {
			item->setData(0, Qt::DecorationRole, QIcon(":/share/images/database.png"));
			loadSchemes (parent->parent ()->text(0) + "."  + item->text(0), item);
		} else {
			item->setData(0, Qt::DecorationRole, QIcon(":/share/images/disconnected-database.png"));
		}
	}
}

void DatabaseTree::loadSchemes (const QString& connectionName, QTreeWidgetItem *parent)
{
	QSqlQuery query (QSqlDatabase::database(connectionName));

	if (!query.exec("SELECT nspname, oid FROM pg_namespace ORDER BY 1")) {
		QMessageBox::critical(this, "", query.lastError().text());
		return;
	}

	QTreeWidgetItem *item = 0;

	const QList<QTreeWidgetItem*> l = tree->findItems(tr ("Schemes"), Qt::MatchFixedString | Qt::MatchRecursive, 0);
	foreach (QTreeWidgetItem *i, l) {
		if (i->parent() == parent) {
			item = i;
			break;
		}
	}
	if (!item) {
		item = new QTreeWidgetItem ();
		item->setText(0, tr ("Schemes"));
		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		
		parent->addChild(item);
	}
	parent = item;

	while (query.next()) {
		item = 0;
		const QList<QTreeWidgetItem*> l = tree->findItems(query.value(0).toString(), Qt::MatchFixedString | Qt::MatchRecursive, 0);
		foreach (QTreeWidgetItem *i, l) {
			if (i->parent() == parent) {
				item = i;
				break;
			}
		}
		
		if (!item) {
			item = new QTreeWidgetItem ();
			item->setText(0, query.value(0).toString());
			item->setData(0, Qt::UserRole, "SCHEME");
			item->setData(0, Qt::UserRole + 1, query.value(1));
			item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

			parent->addChild(item);
		}
		loadTables (connectionName, item);
		loadViews (connectionName, item);
		loadSequences (connectionName, item);
	}
}

void DatabaseTree::loadTables (const QString& connectionName, QTreeWidgetItem *parent)
{
	QSqlQuery query (QSqlDatabase::database(connectionName));
	query.prepare("SELECT relname FROM pg_class WHERE relkind='r' AND relnamespace=:relnamespace ORDER BY 1");
	query.bindValue (":relnamespace", parent->data (0, Qt::UserRole + 1).toInt ());

	if (!query.exec ()) {
		QMessageBox::critical(this, "", query.lastError().text());
		return;
	}

	QTreeWidgetItem *item = 0;

	const QList<QTreeWidgetItem*> l = tree->findItems(tr ("Tables"), Qt::MatchFixedString | Qt::MatchRecursive, 0);
	foreach (QTreeWidgetItem *i, l) {
		if (i->parent() == parent) {
			item = i;
			break;
		}
	}
	if (!item) {
		item = new QTreeWidgetItem ();
		item->setText(0, tr ("Tables"));
		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		item->setData (0, Qt::DecorationRole, QIcon (":/share/images/table.png"));
		
		parent->addChild(item);
	}
	parent = item;

	while (query.next()) {
		item = 0;
		const QList<QTreeWidgetItem*> l = tree->findItems(query.value(0).toString(), Qt::MatchFixedString | Qt::MatchRecursive, 0);
		foreach (QTreeWidgetItem *i, l) {
			if (i->parent() == parent) {
				item = i;
				break;
			}
		}
		
		if (!item) {
			item = new QTreeWidgetItem ();
			item->setText(0, query.value(0).toString());
			item->setData(0, Qt::UserRole, "TABLE");
			item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
			item->setData (0, Qt::DecorationRole, QIcon (":/share/images/table.png"));

			parent->addChild(item);
		}
	}
}

void DatabaseTree::loadViews (const QString& connectionName, QTreeWidgetItem *parent)
{
	QSqlQuery query (QSqlDatabase::database(connectionName));
	query.prepare("SELECT relname FROM pg_class WHERE relkind='v' AND relnamespace=:relnamespace ORDER BY 1");
	query.bindValue (":relnamespace", parent->data (0, Qt::UserRole + 1).toInt ());

	if (!query.exec ()) {
		QMessageBox::critical(this, "", query.lastError().text());
		return;
	}

	QTreeWidgetItem *item = 0;

	const QList<QTreeWidgetItem*> l = tree->findItems(tr ("Views"), Qt::MatchFixedString | Qt::MatchRecursive, 0);
	foreach (QTreeWidgetItem *i, l) {
		if (i->parent() == parent) {
			item = i;
			break;
		}
	}
	if (!item) {
		item = new QTreeWidgetItem ();
		item->setText(0, tr ("Views"));
		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		item->setData (0, Qt::DecorationRole, QIcon (":/share/images/view.png"));
		
		parent->addChild(item);
	}
	parent = item;

	while (query.next()) {
		item = 0;
		const QList<QTreeWidgetItem*> l = tree->findItems(query.value(0).toString(), Qt::MatchFixedString | Qt::MatchRecursive, 0);
		foreach (QTreeWidgetItem *i, l) {
			if (i->parent() == parent) {
				item = i;
				break;
			}
		}
		
		if (!item) {
			item = new QTreeWidgetItem ();
			item->setText(0, query.value(0).toString());
			item->setData(0, Qt::UserRole, "VIEW");
			item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
			item->setData (0, Qt::DecorationRole, QIcon (":/share/images/view.png"));

			parent->addChild(item);
		}
	}
}

void DatabaseTree::loadSequences (const QString& connectionName, QTreeWidgetItem *parent)
{
	QSqlQuery query (QSqlDatabase::database(connectionName));
	query.prepare("SELECT relname FROM pg_class WHERE relkind='S' AND relnamespace=:relnamespace ORDER BY 1");
	query.bindValue (":relnamespace", parent->data (0, Qt::UserRole + 1).toInt ());

	if (!query.exec ()) {
		QMessageBox::critical(this, "", query.lastError().text());
		return;
	}

	QTreeWidgetItem *item = 0;

	const QList<QTreeWidgetItem*> l = tree->findItems(tr ("Sequences"), Qt::MatchFixedString | Qt::MatchRecursive, 0);
	foreach (QTreeWidgetItem *i, l) {
		if (i->parent() == parent) {
			item = i;
			break;
		}
	}
	if (!item) {
		item = new QTreeWidgetItem ();
		item->setText(0, tr ("Sequences"));
		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		item->setData (0, Qt::DecorationRole, QIcon (":/share/images/sequence.png"));
		
		parent->addChild(item);
	}
	parent = item;

	while (query.next()) {
		item = 0;
		const QList<QTreeWidgetItem*> l = tree->findItems(query.value(0).toString(), Qt::MatchFixedString | Qt::MatchRecursive, 0);
		foreach (QTreeWidgetItem *i, l) {
			if (i->parent() == parent) {
				item = i;
				break;
			}
		}
		
		if (!item) {
			item = new QTreeWidgetItem ();
			item->setText(0, query.value(0).toString());
			item->setData(0, Qt::UserRole, "SEQUENCES");
			item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
			item->setData (0, Qt::DecorationRole, QIcon (":/share/images/sequence.png"));

			parent->addChild(item);
		}
	}
}

void DatabaseTree::itemExpanded (QTreeWidgetItem *item)
{
	if (item->data(0, Qt::UserRole).toString() == "CONNECTION") {
		if (!QSqlDatabase::database(item->text(0)).isOpen()) {
			const Connection& c = connections.at (item->data(1, Qt::UserRole).toInt());

			QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", c.name);

			db.setHostName(c.host);
			db.setPort(c.port);
			db.setUserName(c.userName);
			db.setPassword(c.password);

			if (!db.open()) {
				QMessageBox::critical(this, "", db.lastError().text());
				QSqlDatabase::removeDatabase(c.name);
			} else {
				loadTree ();
			}
		}
	}
	if (item->data(0, Qt::UserRole).toString() == "DATABASE") {
		const Connection& c = connections.at (item->parent()->parent ()->data(1, Qt::UserRole).toInt());
		if (!QSqlDatabase::database(c.name + "." + item->text (0)).isOpen()) {
			QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", c.name + "."  + item->text (0));

			db.setHostName(c.host);
			db.setPort(c.port);
			db.setUserName(c.userName);
			db.setPassword(c.password);
			db.setDatabaseName (item->text (0));

			if (!db.open()) {
				QMessageBox::critical(this, "", db.lastError().text());
				QSqlDatabase::removeDatabase(c.name + "."  + item->text (0));
			} else {
				loadTree ();
			}
		}
	}
}

void DatabaseTree::itemActivated (QTreeWidgetItem *item, int column)
{
	if (item->data(0, Qt::UserRole).toString() == "CONNECTION") {
		int i = item->data(1, Qt::UserRole).toInt();
		ConnectionDialog d (this);

		d.setConnectionName(connections[i].name);
		d.setHost(connections[i].host);
		d.setPort(connections[i].port);
		d.setUserName(connections[i].userName);
		d.setPassword(connections[i].password);

		if (d.exec()) {
			connections[i].name = d.connectionName();
			connections[i].host = d.host();
			connections[i].port = d.port();
			connections[i].userName = d.userName();
			connections[i].password = d.password();
		}
	}
	if (item->data(0, Qt::UserRole).toString() == "TABLE") {
		QTreeWidgetItem *parent = item;
		
		while (parent->data(0, Qt::UserRole).toString() != "DATABASE") {
			parent = parent->parent ();
		}
		emit openTable (parent->parent ()->parent ()->text (0) + "." + parent->text (0), item->text (0));
	}
}
