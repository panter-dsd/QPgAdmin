#include <QtGui/QTableView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QToolBar>
#include <QtGui/QAction>
#include <QtGui/QtEvents>

#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>

#include "edittablewidget.h"

EditTableWidget::EditTableWidget (const QString& connectionName, const QString& tableName, QWidget *parent)
	: QWidget (parent)
{
	model = new QSqlTableModel (this, QSqlDatabase::database (connectionName));
	model->setEditStrategy(QSqlTableModel::OnManualSubmit);
	model->setTable (tableName);
	model->select ();

	view = new QTableView (this);
	view->setModel (model);
	view->setContextMenuPolicy (Qt::ActionsContextMenu);
	view->setSortingEnabled (true);

	toolBar = new QToolBar (this);

	QVBoxLayout *mainLayout = new QVBoxLayout ();
	mainLayout->setContentsMargins (0, 0, 0, 0);
	mainLayout->addWidget (toolBar);
	mainLayout->addWidget (view);
	setLayout (mainLayout);

	actionSave = new QAction (this);
	actionSave->setObjectName ("SAVE");
	actionSave->setShortcut (QKeySequence::Save);
	actionSave->setIcon(QIcon(":/share/images/save.png"));
	connect (actionSave, SIGNAL (triggered ()), model, SLOT (submitAll ()));
	toolBar->addAction (actionSave);

	actionRevert = new QAction (this);
	actionRevert->setObjectName ("REVERT");
	actionRevert->setIcon(QIcon(":/share/images/undo.png"));
	connect (actionRevert, SIGNAL (triggered ()), model, SLOT (revertAll ()));
	toolBar->addAction (actionRevert);

	actionAddIncludeFilter = new QAction (this);
	actionAddIncludeFilter->setObjectName ("ADD_INCLUDE_FILTER");
	connect (actionAddIncludeFilter, SIGNAL (triggered ()), this, SLOT (addIncludeFilter ()));
	view->addAction (actionAddIncludeFilter);

	actionAddExcludeFilter = new QAction (this);
	actionAddExcludeFilter->setObjectName ("ADD_INCLUDE_FILTER");
	connect (actionAddExcludeFilter, SIGNAL (triggered ()), this, SLOT (addExcludeFilter ()));
	view->addAction (actionAddExcludeFilter);

	retranslateStrings ();
}

EditTableWidget::~EditTableWidget ()
{

}

bool EditTableWidget::event(QEvent *ev)
{
	if (ev->type() == QEvent::LanguageChange) {
		retranslateStrings();
	}

	return QWidget::event(ev);
}

void EditTableWidget::retranslateStrings()
{
	setWindowTitle (tr ("Edit table") + " " + model->tableName ());
	actionSave->setText (tr ("Save"));
	actionRevert->setText (tr ("Revert"));
	actionAddIncludeFilter->setText (tr ("Add include filter"));
	actionAddExcludeFilter->setText (tr ("Add exclude filter"));
}

QString EditTableWidget::dataForFilter (const QModelIndex& index)
{
	const QSqlRecord& record = model->record ();
	QString data = index.data (Qt::EditRole).toString ();
	switch (record.field (index.column ()).type ()) {
	case QVariant::String: case QVariant::Date: case QVariant::Time: case QVariant::DateTime:
		data = "'" + data + "'";
		break;
	default:
		break;
	}
	return data;
}

void EditTableWidget::addIncludeFilter ()
{
	const QString& data = dataForFilter (view->currentIndex ());
	const QString& column =  model->record ().fieldName (view->currentIndex ().column ());

	QString filter = model->filter ();

	if (!filter.isEmpty ())
		filter += " AND ";

	if (view->currentIndex ().data (Qt::EditRole).isNull ()) {
		model->setFilter (filter + column + " IS NULL");
	} else {
		model->setFilter (filter + column + "=" + data);
	}

	model->select ();
}

void EditTableWidget::addExcludeFilter ()
{
	const QString& data = dataForFilter (view->currentIndex ());
	const QString& column =  model->record ().fieldName (view->currentIndex ().column ());

	QString filter = model->filter ();

	if (!filter.isEmpty ())
		filter += " AND ";

	if (view->currentIndex ().data (Qt::EditRole).isNull ()) {
		model->setFilter (filter + column + " IS NOT NULL");
	} else {
		model->setFilter (filter + column + " IS DISTINCT FROM " + data);
	}
	model->select ();
}
