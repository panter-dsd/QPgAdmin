#include <QtCore/QSettings>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDir>

#include <QtGui/QTabWidget>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QTableView>
#include <QtGui/QToolBar>
#include <QtGui/QAction>
#include <QtGui/QVBoxLayout>
#include <QtGui/QSplitter>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QComboBox>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlError>

#include "sqlquerywidget.h"
#include "querythread.h"
#include "sqlhighlighter.h"

SqlQueryWidget::SqlQueryWidget (const QString& connectionName, QWidget *parent)
	: QWidget (parent), m_connectionName (connectionName)
{
	inputTabs = new QTabWidget (this);
	inputTabs->setContextMenuPolicy (Qt::ActionsContextMenu);
	inputTabs->setTabsClosable (true);
	connect (inputTabs, SIGNAL (currentChanged (int)), this, SLOT (updateActions ()));
	connect (inputTabs, SIGNAL (tabCloseRequested (int)), this, SLOT (closeTab (int)));

	outputTabs = new QTabWidget (this);

	outputModel = new QSqlQueryModel (this);

	outputTable = new QTableView (this);
	outputTable->setModel (outputModel);
	outputTabs->addTab (outputTable, "");

	messagesEdit = new QPlainTextEdit (this);
	messagesEdit->setReadOnly (true);
	outputTabs->addTab (messagesEdit, "");

	toolBar = new QToolBar (this);

	splitter = new QSplitter (Qt::Vertical, this);
	splitter->setObjectName ("SPLITTER");
	splitter->addWidget (inputTabs);
	splitter->addWidget (outputTabs);

	QVBoxLayout *mainLayout = new QVBoxLayout ();
	mainLayout->setContentsMargins (0, 0, 0, 0);
	mainLayout->addWidget (toolBar);
	mainLayout->addWidget (splitter);
	setLayout (mainLayout);

	connectionEdit = new QComboBox (this);
	connectionEdit->addItems (QSqlDatabase::connectionNames ());
	connectionEdit->setCurrentIndex (connectionEdit->findText (connectionName, Qt::MatchFixedString));

	actionAddSqlEditor = new QAction (this);
	actionAddSqlEditor->setIcon (QIcon (":/share/images/add.png"));
	connect (actionAddSqlEditor, SIGNAL (triggered ()), this, SLOT (addSqlEditor ()));
	inputTabs->addAction (actionAddSqlEditor);
	toolBar->addAction (actionAddSqlEditor);

	actionOpen = new QAction (this);
	actionOpen->setIcon (QIcon (":/share/images/open.png"));
	actionOpen->setShortcut (QKeySequence::Open);
	connect (actionOpen, SIGNAL (triggered ()), this, SLOT (open ()));
	toolBar->addAction (actionOpen);

	actionSave = new QAction (this);
	actionSave->setIcon (QIcon (":/share/images/save.png"));
	actionSave->setShortcut (QKeySequence::Save);
	connect (actionSave, SIGNAL (triggered ()), this, SLOT (save ()));
	toolBar->addAction (actionSave);

	actionSaveAs = new QAction (this);
	actionSaveAs->setIcon (QIcon (":/share/images/save_as.png"));
	actionSaveAs->setShortcut (Qt::CTRL + Qt::SHIFT + Qt::Key_S);
	connect (actionSaveAs, SIGNAL (triggered ()), this, SLOT (saveAs ()));
	toolBar->addAction (actionSaveAs);

	toolBar->addSeparator ();

	actionStart = new QAction (this);
	actionStart->setIcon (QIcon (":/share/images/start.png"));
	actionStart->setShortcut (Qt::Key_F5);
	connect (actionStart, SIGNAL (triggered ()), this, SLOT (start ()));
	toolBar->addAction (actionStart);

	actionStop = new QAction (this);
	actionStop->setIcon (QIcon (":/share/images/stop.png"));
	actionStop->setEnabled (false);
	toolBar->addAction (actionStop);

	toolBar->addSeparator ();
	toolBar->addWidget (connectionEdit);

	loadSettings();
	retranslateStrings ();
	addSqlEditor ();
}

SqlQueryWidget::~SqlQueryWidget()
{
	saveSettings();
}

void SqlQueryWidget::retranslateStrings()
{
	setWindowTitle (tr ("SQL editor"));
	updateTabCaptions ();
	outputTabs->setTabText (outputTabs->indexOf (outputTable), tr ("Output table"));
	outputTabs->setTabText (outputTabs->indexOf (messagesEdit), tr ("Messages"));

	actionAddSqlEditor->setText (tr ("Add SQL editor"));
	actionOpen->setText (tr ("Open"));
	actionSave->setText (tr ("Save"));
	actionSaveAs->setText (tr ("Save as..."));
	actionStart->setText (tr ("Start"));
	actionStop->setText (tr ("Stop"));
}

void SqlQueryWidget::loadSettings()
{
	QSettings settings;

	settings.beginGroup("SqlQueryWidget");
	splitter->restoreState(settings.value("State", "").toByteArray());
	settings.endGroup();
}

void SqlQueryWidget::saveSettings()
{
	QSettings settings;

	settings.beginGroup("SqlQueryWidget");
	settings.setValue("State", splitter->saveState());
	settings.endGroup();

	settings.sync();
}

bool SqlQueryWidget::event(QEvent *ev)
{
	if (ev->type() == QEvent::LanguageChange) {
		retranslateStrings();
	}
	if (ev->type () == QEvent::Close) {
		while (inputTabs->count () > 0) {
			if (!closeTab (0)) {
				ev->ignore ();
				return false;
			}
		}
	}

	return QWidget::event(ev);
}

QPlainTextEdit* SqlQueryWidget::addSqlEditor ()
{
	QPlainTextEdit *e = new QPlainTextEdit (this);
	connect (e, SIGNAL (modificationChanged (bool)), this, SLOT (updateTabCaptions ()));
	sqlEdits << e;

	SQLHighlighter *sqlhighlighter = new SQLHighlighter(e->document()); 

	const int index = inputTabs->addTab (e, tr ("Unnamed"));
	inputTabs->setCurrentIndex (index);
	return e;
}

void SqlQueryWidget::open ()
{
	QSettings settings;
	
	const QStringList& fileNames = QFileDialog::getOpenFileNames (this,
															tr ("Open file"),
															settings.value ("SqlQueryWidget/OpenPath", "").toString (),
															tr("Sql files (*.sql)\nAll files (*.*)"));
	if (fileNames.isEmpty ()) {
		return;
	}

	settings.setValue ("SqlQueryWidget/OpenPath", QFileInfo (fileNames.first ()).absolutePath ());
	settings.sync ();

	foreach (const QString& fileName, fileNames) {
		QFile file (fileName);
		if (!file.open (QIODevice::ReadOnly)) {
			QMessageBox::critical (this, "", tr ("Error open file"));
			return;
		}
		QTextStream stream (&file);
		
		QPlainTextEdit *e = addSqlEditor ();
		e->setPlainText (stream.readAll ());
		e->document ()->setModified (false);
		e->setObjectName (QFileInfo (fileName).absoluteFilePath ());
		file.close ();
	}
	updateTabCaptions ();
}

bool SqlQueryWidget::save ()
{
	QPlainTextEdit *e = qobject_cast<QPlainTextEdit*> (inputTabs->currentWidget ());
	if (!e)
		return false;

	const QString& fileName = e->objectName ();
	if (fileName.isEmpty ()) {
		return saveAs ();
	}	

	QFile file (fileName);
	if (!file.open (QIODevice::WriteOnly)) {
		QMessageBox::critical (this, "", tr ("Error save file"));
		return false;
	}
	QTextStream stream (&file);

	stream << e->toPlainText ();
	file.close ();
	e->document ()->setModified (false);
	updateTabCaptions ();
	return true;
}

bool SqlQueryWidget::saveAs ()
{
	QPlainTextEdit *e = qobject_cast<QPlainTextEdit*> (inputTabs->currentWidget ());
	if (!e)
		return false;

	QSettings settings;
	const QString& fileName = QFileDialog::getSaveFileName (this,
															tr ("Save"),
															settings.value ("SqlQueryWidget/SavePath", "").toString (),
															tr("Sql files (*.sql)\nAll files (*.*)"));
	if (fileName.isEmpty ()) {
		return false;
	}

	settings.setValue ("SqlQueryWidget/SavePath", QFileInfo (fileName).absolutePath ());
	settings.sync ();

	e->setObjectName (QFileInfo (fileName).absoluteFilePath ());
	updateTabCaptions ();
	return save ();
}

void SqlQueryWidget::updateTabCaptions ()
{
	for (int i = 0, count = inputTabs->count (); i < count; i++) {
		QPlainTextEdit *e = qobject_cast<QPlainTextEdit*> (inputTabs->widget (i));
		if (!e)
			return;
	
		if (e->objectName ().isEmpty ()) {
			QString text = tr ("Unnamed");
			if (e->document ()->isModified ())
				text += " *";
			inputTabs->setTabText (i, text);
		} else {
			QFileInfo fi (e->objectName ());
			QString text = fi.fileName ();
			if (e->document ()->isModified ())
				text += " *";
			inputTabs->setTabText (i, text);  
			inputTabs->setTabToolTip (i, QDir::toNativeSeparators (fi.absoluteFilePath ()));  
		}
	}
	updateActions ();
}

void SqlQueryWidget::start ()
{
	if (connectionEdit->currentIndex () < 0) {
		QMessageBox::critical (this, "", tr ("Choose connection"));
		return;
	}

	QPlainTextEdit *e = qobject_cast<QPlainTextEdit*> (inputTabs->currentWidget ());
	if (!e)
		return;

	actionStart->setEnabled (false);
	actionStop->setEnabled (true);
	QueryThread *thread = new QueryThread (connectionEdit->currentText (), e->toPlainText (), this);
	connect (thread, SIGNAL (finished ()), this, SLOT (queryFinished ()));
	connect (actionStop, SIGNAL (triggered ()), thread, SLOT (terminate ()));
	thread->start ();
	m_time.start ();
}

void SqlQueryWidget::queryFinished ()
{
	actionStart->setEnabled (true);
	actionStop->setEnabled (false);

	QueryThread *thread = qobject_cast <QueryThread*> (sender ());
	if (!thread)
		return;
	QSqlQuery query = thread->lastQuery ();
	thread->deleteLater ();
	outputModel->setQuery (query);
	messagesEdit->setPlainText (query.lastQuery ());
	if (query.lastError ().isValid ()) {
		messagesEdit->appendPlainText (query.lastError ().text ());
		outputTabs->setCurrentWidget (messagesEdit);
	} else {
		messagesEdit->appendPlainText (tr ("The query is successfully comlete for %1 secs").arg (m_time.elapsed () / 100));
	}
}

void SqlQueryWidget::connectionsChanged ()
{
	const QString& currentConnection = connectionEdit->currentText ();
	connectionEdit->clear ();
	connectionEdit->addItems (QSqlDatabase::connectionNames ());
	connectionEdit->setCurrentIndex (connectionEdit->findText (currentConnection, Qt::MatchFixedString));
}

void SqlQueryWidget::updateActions ()
{
	QPlainTextEdit *e = qobject_cast<QPlainTextEdit*> (inputTabs->currentWidget ());
	if (!e)
		return;

	actionSave->setEnabled (e->document ()->isModified () || e->objectName ().isEmpty ());
}

bool SqlQueryWidget::closeTab (int index)
{
	inputTabs->setCurrentIndex (index);

	QPlainTextEdit *e = qobject_cast<QPlainTextEdit*> (inputTabs->widget (index));
	if (!e)
		return false;

	if (e->document ()->isModified ()) {
		int res = QMessageBox::question (this, "", tr ("Tab \"%1\" is modified.\nSave?").arg (inputTabs->tabText (index)), 
										 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		
		if (res == QMessageBox::Cancel) {
			return false;
		}
	
		if (res == QMessageBox::Yes) { 
			if (!save ())
				return false;
		}
	}

	delete e;
	return true;
}
