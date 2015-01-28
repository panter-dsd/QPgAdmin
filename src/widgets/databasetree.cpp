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

DatabaseTree::DatabaseTree(QWidget *parent)
	: QWidget(parent)
{
	setObjectName("DATABASE_TREE");

	tree = new QTreeWidget(this);
	tree->header()->hide();
	tree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(tree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(treeContextMenu(QPoint)));
	connect(tree, SIGNAL(itemExpanded(QTreeWidgetItem *)), this, SLOT(itemExpanded(QTreeWidgetItem *)));
	connect(tree, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(itemActivated(QTreeWidgetItem *, int)));

	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addWidget(tree);
	setLayout(mainLayout);

	actionAddConnection = new QAction(this);
	connect(actionAddConnection, SIGNAL(triggered()), this, SLOT(addConnection()));

	actionEditConnection = new QAction(this);
	connect(actionEditConnection, SIGNAL(triggered()), this, SLOT(editConnection()));

	actionCloseConnection = new QAction(this);
	connect(actionCloseConnection, SIGNAL(triggered()), this, SLOT(closeConnection()));

	retranslateStrings();
	loadSettings();
	loadTree();
}

DatabaseTree::~DatabaseTree()
{
	saveSettings();
	foreach(const QString & connectionName, QSqlDatabase::connectionNames()) {
		QSqlDatabase::database(connectionName).close();
		QSqlDatabase::removeDatabase(connectionName);
	}
}

QString DatabaseTree::currentConnection() const
{
	QTreeWidgetItem *item = tree->currentItem();
	const QMap <QString, QVariant> &option = item->data(0, Qt::UserRole).value<QMap <QString, QVariant>> ();
	return option.value("ConnectionName").toString();
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
		c.maintenanceBase = settings.value("MaintenanceBase").toString();
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
		settings.setValue("MaintenanceBase", connections.at(i).maintenanceBase);
		settings.setValue("UserName", connections.at(i).userName);
		settings.setValue("Password", connections.at(i).password);
	}
	settings.endArray();
}

void DatabaseTree::retranslateStrings()
{
	setWindowTitle(tr("Database tree"));
	actionAddConnection->setText(tr("Add connection"));
	actionEditConnection->setText(tr("Edit connection"));
	actionCloseConnection->setText(tr("Close connection"));
}

void DatabaseTree::addConnection()
{
	ConnectionDialog d(this);
	if (d.exec()) {
		Connection c;
		c.name = d.connectionName();
		c.host = d.host();
		c.port = d.port();
		c.maintenanceBase = d.maintenanceBase();
		c.userName = d.userName();
		c.password = d.password();
		connections.append(c);

		loadTree();
	}
}

void DatabaseTree::editConnection()
{
	QAction *action = qobject_cast <QAction *> (sender());
	if (!action)
		return;

	const int index = action->data().toInt();

	ConnectionDialog d(this);
	d.setConnectionName(connections.at(index).name);
	d.setHost(connections.at(index).host);
	d.setPort(connections.at(index).port);
	d.setMaintenanceBase(connections.at(index).maintenanceBase);
	d.setUserName(connections.at(index).userName);
	d.setPassword(connections.at(index).password);

	if (d.exec()) {
		connections [index].name = d.connectionName();
		connections [index].host = d.host();
		connections [index].port = d.port();
		connections [index].maintenanceBase = d.maintenanceBase();
		connections [index].userName = d.userName();
		connections [index].password = d.password();

		loadTree();
	}
}

void DatabaseTree::closeConnection()
{
	QAction *action = qobject_cast <QAction *> (sender());
	if (!action)
		return;

	const QString &connectionName = action->data().toString();

	foreach(const QString & name, QSqlDatabase::connectionNames()) {
		if (name.startsWith(connectionName)) {
			QSqlDatabase::database(name, false).close();
			if (QSqlDatabase::database(name, false).isOpen()) {
				QMessageBox::critical(this, "", QSqlDatabase::database(name, false).lastError().text());
				return;
			}
			QSqlDatabase::removeDatabase(name);
		}
	}

	emit connectionsChanged();
	loadTree();
}

void DatabaseTree::treeContextMenu(const QPoint &point)
{
	QMenu menu;

	QTreeWidgetItem *item = tree->itemAt(point);
	if (!item) {
		menu.addAction(actionAddConnection);
	}
	else {
		const QMap <QString, QVariant> &option = item->data(0, Qt::UserRole).value<QMap <QString, QVariant>> ();
		if (option ["Type"].toString() == "Connection") {
			actionEditConnection->setData(option ["Index"]);
			menu.addAction(actionEditConnection);
			if (QSqlDatabase::contains(item->text(0))) {
				actionCloseConnection->setData(item->text(0));
				menu.addAction(actionCloseConnection);
			}
		}
		if (option ["Type"].toString() == "Database") {
			const QString &connectionName = option ["ConnectionName"].toString() + "." + item->text(0);
			if (QSqlDatabase::contains(connectionName)) {
				actionCloseConnection->setData(connectionName);
				menu.addAction(actionCloseConnection);
			}
		}
	}

	if (!menu.actions().isEmpty()) {
		menu.exec(tree->mapToGlobal(point));
	}
}

void DatabaseTree::loadTree()
{
	QMap<QString, QVariant> options;
	options ["Type"] = "Connection";

	QTreeWidgetItem *item;

	for (int i = 0, size = connections.size(); i < size; i++) {

		item = 0;
		const QList<QTreeWidgetItem *> l = tree->findItems(connections.at(i).name, Qt::MatchFixedString, 0);
		foreach(QTreeWidgetItem * i, l) {
			if (i->parent() == 0) {
				item = i;
				break;
			}
		}

		if (!item) {
			item = new QTreeWidgetItem();
			item->setText(0, connections.at(i).name);
			options ["Index"] = i;
			item->setData(0, Qt::UserRole, options);

			tree->addTopLevelItem(item);
		}

		if (QSqlDatabase::database(item->text(0), false).isOpen()) {
			loadDatabases(item);
			item->setData(0, Qt::DecorationRole, QIcon(":/share/images/connect_established.png"));
		}
		else {
			item->setData(0, Qt::DecorationRole, QIcon(":/share/images/connect_no.png"));
			item->setExpanded(false);
			qDeleteAll(item->takeChildren());
			item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		}
	}
}

void DatabaseTree::loadDatabases(QTreeWidgetItem *parent)
{
	QMap<QString, QVariant> options = parent->data(0, Qt::UserRole).value<QMap<QString, QVariant>> ();
	options ["Type"] = "Database";
	options ["ConnectionName"] = parent->text(0);

	QSqlQuery query(QSqlDatabase::database(parent->text(0)));
	if (!query.exec("SELECT datname FROM pg_database")) {
		QMessageBox::critical(this, "", query.lastError().text());
		return;
	}

	QTreeWidgetItem *item = 0;
	const QList<QTreeWidgetItem *> l = tree->findItems(tr("Databases"), Qt::MatchFixedString | Qt::MatchRecursive, 0);
	foreach(QTreeWidgetItem * i, l) {
		if (i->parent() == parent) {
			item = i;
			break;
		}
	}
	if (!item) {
		item = new QTreeWidgetItem();
		item->setText(0, tr("Databases"));

		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

		parent->addChild(item);
	}
	parent = item;

	while (query.next()) {
		item = 0;
		const QList<QTreeWidgetItem *> l = tree->findItems(query.value(0).toString(), Qt::MatchFixedString | Qt::MatchRecursive, 0);
		foreach(QTreeWidgetItem * i, l) {
			if (i->parent() == parent) {
				item = i;
				break;
			}
		}

		if (!item) {
			item = new QTreeWidgetItem();
			item->setText(0, query.value(0).toString());
			item->setData(0, Qt::UserRole, options);

			parent->addChild(item);
		}
		if (QSqlDatabase::database(options ["ConnectionName"].toString() + "." + item->text(0), false).isOpen()) {
			item->setData(0, Qt::DecorationRole, QIcon(":/share/images/database.png"));
			loadSchemes(item);
		}
		else {
			item->setData(0, Qt::DecorationRole, QIcon(":/share/images/disconnected-database.png"));
			item->setExpanded(false);
			qDeleteAll(item->takeChildren());
			item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		}
	}
}

void DatabaseTree::loadSchemes(QTreeWidgetItem *parent)
{
	QMap<QString, QVariant> options = parent->data(0, Qt::UserRole).value <QMap<QString, QVariant>> ();
	options ["Type"] = "Scheme";
	options ["ConnectionName"] = options ["ConnectionName"].toString() + "." + parent->text(0);

	QSqlQuery query(QSqlDatabase::database(options ["ConnectionName"].toString()));

	if (!query.exec("SELECT nspname, oid FROM pg_namespace ORDER BY 1")) {
		QMessageBox::critical(this, "", query.lastError().text());
		return;
	}

	QTreeWidgetItem *item = 0;

	const QList<QTreeWidgetItem *> l = tree->findItems(tr("Schemes"), Qt::MatchFixedString | Qt::MatchRecursive, 0);
	foreach(QTreeWidgetItem * i, l) {
		if (i->parent() == parent) {
			item = i;
			break;
		}
	}
	if (!item) {
		item = new QTreeWidgetItem();
		item->setText(0, tr("Schemes"));
		item->setData(0, Qt::UserRole, options);
		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

		parent->addChild(item);
	}
	parent = item;

	while (query.next()) {
		item = 0;
		const QList<QTreeWidgetItem *> l = tree->findItems(query.value(0).toString(), Qt::MatchFixedString | Qt::MatchRecursive, 0);
		foreach(QTreeWidgetItem * i, l) {
			if (i->parent() == parent) {
				item = i;
				break;
			}
		}

		if (!item) {
			item = new QTreeWidgetItem();
			item->setText(0, query.value(0).toString());
			options ["ID"] = query.value(1);
			options ["Scheme"] = query.value(0).toString();
			item->setData(0, Qt::UserRole, options);
			item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

			parent->addChild(item);
		}
		loadTables(item);
		loadViews(item);
		loadSequences(item);
	}
}

void DatabaseTree::loadTables(QTreeWidgetItem *parent)
{
	QMap<QString, QVariant> options = parent->data(0, Qt::UserRole).value<QMap<QString, QVariant>> ();
	options ["Type"] = "Table";

	QSqlQuery query(QSqlDatabase::database(options ["ConnectionName"].toString()));
	query.prepare("SELECT relname FROM pg_class WHERE relkind='r' AND relnamespace=:relnamespace ORDER BY 1");
	query.bindValue(":relnamespace", options ["ID"].toInt());

	if (!query.exec()) {
		QMessageBox::critical(this, "", query.lastError().text());
		return;
	}

	QTreeWidgetItem *item = 0;

	const QList<QTreeWidgetItem *> l = tree->findItems(tr("Tables"), Qt::MatchFixedString | Qt::MatchRecursive, 0);
	foreach(QTreeWidgetItem * i, l) {
		if (i->parent() == parent) {
			item = i;
			break;
		}
	}
	if (!item) {
		item = new QTreeWidgetItem();
		item->setText(0, tr("Tables"));
		item->setData(0, Qt::UserRole, options);
		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		item->setData(0, Qt::DecorationRole, QIcon(":/share/images/table.png"));

		parent->addChild(item);
	}
	parent = item;

	while (query.next()) {
		item = 0;
		const QList<QTreeWidgetItem *> l = tree->findItems(query.value(0).toString(), Qt::MatchFixedString | Qt::MatchRecursive, 0);
		foreach(QTreeWidgetItem * i, l) {
			if (i->parent() == parent) {
				item = i;
				break;
			}
		}

		if (!item) {
			item = new QTreeWidgetItem();
			item->setText(0, query.value(0).toString());
			item->setData(0, Qt::UserRole, options);
			item->setData(0, Qt::DecorationRole, QIcon(":/share/images/table.png"));

			parent->addChild(item);
		}
	}
}

void DatabaseTree::loadViews(QTreeWidgetItem *parent)
{
	QMap<QString, QVariant> options = parent->data(0, Qt::UserRole).value<QMap<QString, QVariant>> ();
	options ["Type"] = "View";

	QSqlQuery query(QSqlDatabase::database(options ["ConnectionName"].toString()));
	query.prepare("SELECT relname FROM pg_class WHERE relkind='v' AND relnamespace=:relnamespace ORDER BY 1");
	query.bindValue(":relnamespace", options ["ID"].toInt());

	if (!query.exec()) {
		QMessageBox::critical(this, "", query.lastError().text());
		return;
	}

	QTreeWidgetItem *item = 0;

	const QList<QTreeWidgetItem *> l = tree->findItems(tr("Views"), Qt::MatchFixedString | Qt::MatchRecursive, 0);
	foreach(QTreeWidgetItem * i, l) {
		if (i->parent() == parent) {
			item = i;
			break;
		}
	}
	if (!item) {
		item = new QTreeWidgetItem();
		item->setText(0, tr("Views"));
		item->setData(0, Qt::UserRole, options);
		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		item->setData(0, Qt::DecorationRole, QIcon(":/share/images/view.png"));

		parent->addChild(item);
	}
	parent = item;

	while (query.next()) {
		item = 0;
		const QList<QTreeWidgetItem *> l = tree->findItems(query.value(0).toString(), Qt::MatchFixedString | Qt::MatchRecursive, 0);
		foreach(QTreeWidgetItem * i, l) {
			if (i->parent() == parent) {
				item = i;
				break;
			}
		}

		if (!item) {
			item = new QTreeWidgetItem();
			item->setText(0, query.value(0).toString());
			item->setData(0, Qt::UserRole, options);
			item->setData(0, Qt::DecorationRole, QIcon(":/share/images/view.png"));

			parent->addChild(item);
		}
	}
}

void DatabaseTree::loadSequences(QTreeWidgetItem *parent)
{
	QMap<QString, QVariant> options = parent->data(0, Qt::UserRole).value<QMap<QString, QVariant>> ();
	options ["Type"] = "Sequence";

	QSqlQuery query(QSqlDatabase::database(options ["ConnectionName"].toString()));
	query.prepare("SELECT relname FROM pg_class WHERE relkind='S' AND relnamespace=:relnamespace ORDER BY 1");
	query.bindValue(":relnamespace", options ["ID"].toInt());

	if (!query.exec()) {
		QMessageBox::critical(this, "", query.lastError().text());
		return;
	}

	QTreeWidgetItem *item = 0;

	const QList<QTreeWidgetItem *> l = tree->findItems(tr("Sequences"), Qt::MatchFixedString | Qt::MatchRecursive, 0);
	foreach(QTreeWidgetItem * i, l) {
		if (i->parent() == parent) {
			item = i;
			break;
		}
	}
	if (!item) {
		item = new QTreeWidgetItem();
		item->setText(0, tr("Sequences"));
		item->setData(0, Qt::UserRole, options);
		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		item->setData(0, Qt::DecorationRole, QIcon(":/share/images/sequence.png"));

		parent->addChild(item);
	}
	parent = item;

	while (query.next()) {
		item = 0;
		const QList<QTreeWidgetItem *> l = tree->findItems(query.value(0).toString(), Qt::MatchFixedString | Qt::MatchRecursive, 0);
		foreach(QTreeWidgetItem * i, l) {
			if (i->parent() == parent) {
				item = i;
				break;
			}
		}

		if (!item) {
			item = new QTreeWidgetItem();
			item->setText(0, query.value(0).toString());
			item->setData(0, Qt::UserRole, options);
			item->setData(0, Qt::DecorationRole, QIcon(":/share/images/sequence.png"));

			parent->addChild(item);
		}
	}
}

void DatabaseTree::itemExpanded(QTreeWidgetItem *item)
{
	QMap<QString, QVariant> options = item->data(0, Qt::UserRole).value<QMap<QString, QVariant>> ();

	if (options ["Type"].toString() == "Connection") {
		if (!QSqlDatabase::database(item->text(0)).isOpen()) {
			const Connection &c = connections.at(options ["Index"].toInt());

			QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", c.name);

			db.setHostName(c.host);
			db.setPort(c.port);
			db.setDatabaseName(c.maintenanceBase);
			db.setUserName(c.userName);
			db.setPassword(c.password);

			if (!db.open()) {
				QMessageBox::critical(this, "", db.lastError().text());
				QSqlDatabase::removeDatabase(c.name);
			}
			else {
				loadTree();
				emit connectionsChanged();
			}
		}
	}
	if (options ["Type"].toString() == "Database") {
		const Connection &c = connections.at(options ["Index"].toInt());
		if (!QSqlDatabase::database(c.name + "." + item->text(0)).isOpen()) {
			QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", c.name + "."  + item->text(0));

			db.setHostName(c.host);
			db.setPort(c.port);
			db.setUserName(c.userName);
			db.setPassword(c.password);
			db.setDatabaseName(item->text(0));

			if (!db.open()) {
				QMessageBox::critical(this, "", db.lastError().text());
				QSqlDatabase::removeDatabase(c.name + "."  + item->text(0));
			}
			else {
				loadTree();
				emit connectionsChanged();
			}
		}
	}
}

void DatabaseTree::itemActivated(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(column)

	QMap<QString, QVariant> options = item->data(0, Qt::UserRole).value<QMap<QString, QVariant>> ();

	if (options ["Type"].toString() == "Table") {
		emit openTable(options ["ConnectionName"].toString(), options ["Scheme"].toString() + "." + item->text(0));
	}
}
