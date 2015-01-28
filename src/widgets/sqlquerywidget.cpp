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
#include <QtGui/QStatusBar>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlError>

#include "sqlquerywidget.h"
#include "querythread.h"
#include "sqlhighlighter.h"

SqlQueryWidget::SqlQueryWidget(const QString &connectionName, QWidget *parent)
	: QWidget(parent), connectionName_(connectionName)
{
	inputTabs_ = new QTabWidget(this);
	inputTabs_->setContextMenuPolicy(Qt::ActionsContextMenu);
	inputTabs_->setTabsClosable(true);
	connect(inputTabs_, SIGNAL(currentChanged(int)), this, SLOT(updateActions()));
	connect(inputTabs_, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

	outputTabs_ = new QTabWidget(this);

	outputModel_ = new QSqlQueryModel(this);

	outputTable_ = new QTableView(this);
	outputTable_->setModel(outputModel_);
	outputTabs_->addTab(outputTable_, "");

	messagesEdit_ = new QPlainTextEdit(this);
	messagesEdit_->setReadOnly(true);
	outputTabs_->addTab(messagesEdit_, "");

	toolBar_ = new QToolBar(this);

	statusBar_ = new QStatusBar(this);

	splitter_ = new QSplitter(Qt::Vertical, this);
	splitter_->setObjectName("SPLITTER");
	splitter_->addWidget(inputTabs_);
	splitter_->addWidget(outputTabs_);

	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addWidget(toolBar_);
	mainLayout->addWidget(splitter_);
	mainLayout->addWidget(statusBar_);
	setLayout(mainLayout);

	connectionEdit_ = new QComboBox(this);
	connectionEdit_->addItems(QSqlDatabase::connectionNames());
	connectionEdit_->setCurrentIndex(connectionEdit_->findText(connectionName, Qt::MatchFixedString));

	actionAddSqlEditor_ = new QAction(this);
	actionAddSqlEditor_->setIcon(QIcon(":/share/images/add.png"));
	connect(actionAddSqlEditor_, SIGNAL(triggered()), this, SLOT(addSqlEditor()));
	inputTabs_->addAction(actionAddSqlEditor_);
	toolBar_->addAction(actionAddSqlEditor_);

	actionOpen_ = new QAction(this);
	actionOpen_->setIcon(QIcon(":/share/images/open.png"));
	actionOpen_->setShortcut(QKeySequence::Open);
	connect(actionOpen_, SIGNAL(triggered()), this, SLOT(open()));
	toolBar_->addAction(actionOpen_);

	actionSave_ = new QAction(this);
	actionSave_->setIcon(QIcon(":/share/images/save.png"));
	actionSave_->setShortcut(QKeySequence::Save);
	connect(actionSave_, SIGNAL(triggered()), this, SLOT(save()));
	toolBar_->addAction(actionSave_);

	actionSaveAs_ = new QAction(this);
	actionSaveAs_->setIcon(QIcon(":/share/images/save_as.png"));
	actionSaveAs_->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_S);
	connect(actionSaveAs_, SIGNAL(triggered()), this, SLOT(saveAs()));
	toolBar_->addAction(actionSaveAs_);

	toolBar_->addSeparator();

	actionUndo_ = new QAction(this);
	actionUndo_->setIcon(QIcon(":/share/images/undo.png"));
	connect(actionUndo_, SIGNAL(triggered()), this, SLOT(undo()));
	toolBar_->addAction(actionUndo_);

	actionRedo_ = new QAction(this);
	actionRedo_->setIcon(QIcon(":/share/images/redo.png"));
	connect(actionRedo_, SIGNAL(triggered()), this, SLOT(redo()));
	toolBar_->addAction(actionRedo_);

	toolBar_->addSeparator();

	actionStart_ = new QAction(this);
	actionStart_->setIcon(QIcon(":/share/images/start.png"));
	actionStart_->setShortcut(Qt::Key_F5);
	connect(actionStart_, SIGNAL(triggered()), this, SLOT(start()));
	toolBar_->addAction(actionStart_);

	actionStop_ = new QAction(this);
	actionStop_->setIcon(QIcon(":/share/images/stop.png"));
	actionStop_->setEnabled(false);
	toolBar_->addAction(actionStop_);

	toolBar_->addSeparator();
	toolBar_->addWidget(connectionEdit_);

	loadSettings();
	retranslateStrings();
	addSqlEditor();
}

SqlQueryWidget::~SqlQueryWidget()
{
	saveSettings();
}

void SqlQueryWidget::retranslateStrings()
{
	setWindowTitle(tr("SQL editor"));
	updateTabCaptions();
	outputTabs_->setTabText(outputTabs_->indexOf(outputTable_), tr("Output table"));
	outputTabs_->setTabText(outputTabs_->indexOf(messagesEdit_), tr("Messages"));

	actionAddSqlEditor_->setText(tr("Add SQL editor"));
	actionOpen_->setText(tr("Open"));
	actionSave_->setText(tr("Save"));
	actionSaveAs_->setText(tr("Save as..."));
	actionUndo_->setText(tr("Undo"));
	actionRedo_->setText(tr("Redo"));
	actionStart_->setText(tr("Start"));
	actionStop_->setText(tr("Stop"));
}

void SqlQueryWidget::loadSettings()
{
	QSettings settings;

	settings.beginGroup("SqlQueryWidget");
	splitter_->restoreState(settings.value("State", "").toByteArray());
	settings.endGroup();
}

void SqlQueryWidget::saveSettings()
{
	QSettings settings;

	settings.beginGroup("SqlQueryWidget");
	settings.setValue("State", splitter_->saveState());
	settings.endGroup();

	settings.sync();
}

bool SqlQueryWidget::event(QEvent *ev)
{
	if (ev->type() == QEvent::LanguageChange) {
		retranslateStrings();
	}
	if (ev->type() == QEvent::Close) {
		while (inputTabs_->count() > 0) {
			if (!closeTab(0)) {
				ev->ignore();
				return false;
			}
		}
	}
	if (ev->type() == QEvent::Timer) {
		const auto elapsed = time_.elapsed();
		statusBar_->showMessage(tr("%1 secs (%2 msecs)").arg(elapsed / 1000).arg(elapsed));
	}

	return QWidget::event(ev);
}

QPlainTextEdit *SqlQueryWidget::addSqlEditor()
{
	QPlainTextEdit *e = new QPlainTextEdit(this);
	connect(e, SIGNAL(modificationChanged(bool)), this, SLOT(updateTabCaptions()));
	sqlEdits_ << e;

	SQLHighlighter *sqlhighlighter = new SQLHighlighter(e->document());
	Q_UNUSED(sqlhighlighter)

	const int index = inputTabs_->addTab(e, tr("Unnamed"));
	inputTabs_->setCurrentIndex(index);
	return e;
}

void SqlQueryWidget::open()
{
	QSettings settings;

	const QStringList &fileNames = QFileDialog::getOpenFileNames(this,
								   tr("Open file"),
								   settings.value("SqlQueryWidget/OpenPath", "").toString(),
								   tr("Sql files (*.sql)\nAll files (*.*)"));
	if (fileNames.isEmpty()) {
		return;
	}

	settings.setValue("SqlQueryWidget/OpenPath", QFileInfo(fileNames.first()).absolutePath());
	settings.sync();

	foreach(const QString & fileName, fileNames) {
		QFile file(fileName);
		if (!file.open(QIODevice::ReadOnly)) {
			QMessageBox::critical(this, "", tr("Error open file"));
			return;
		}
		QTextStream stream(&file);

		QPlainTextEdit *e = addSqlEditor();
		e->setPlainText(stream.readAll());
		e->document()->setModified(false);
		e->setObjectName(QFileInfo(fileName).absoluteFilePath());
		file.close();
	}
	updateTabCaptions();
}

bool SqlQueryWidget::save()
{
	QPlainTextEdit *e = qobject_cast<QPlainTextEdit *> (inputTabs_->currentWidget());
	if (!e)
		return false;

	const QString &fileName = e->objectName();
	if (fileName.isEmpty()) {
		return saveAs();
	}

	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(this, "", tr("Error save file"));
		return false;
	}
	QTextStream stream(&file);

	stream << e->toPlainText();
	file.close();
	e->document()->setModified(false);
	updateTabCaptions();
	return true;
}

bool SqlQueryWidget::saveAs()
{
	QPlainTextEdit *e = qobject_cast<QPlainTextEdit *> (inputTabs_->currentWidget());
	if (!e)
		return false;

	QSettings settings;
	const QString &fileName = QFileDialog::getSaveFileName(this,
							  tr("Save"),
							  settings.value("SqlQueryWidget/SavePath", "").toString(),
							  tr("Sql files (*.sql)\nAll files (*.*)"));
	if (fileName.isEmpty()) {
		return false;
	}

	settings.setValue("SqlQueryWidget/SavePath", QFileInfo(fileName).absolutePath());
	settings.sync();

	e->setObjectName(QFileInfo(fileName).absoluteFilePath());
	updateTabCaptions();
	return save();
}

void SqlQueryWidget::updateTabCaptions()
{
	for (int i = 0, count = inputTabs_->count(); i < count; i++) {
		QPlainTextEdit *e = qobject_cast<QPlainTextEdit *> (inputTabs_->widget(i));
		if (!e)
			return;

		if (e->objectName().isEmpty()) {
			QString text = tr("Unnamed");
			if (e->document()->isModified())
				text += " *";
			inputTabs_->setTabText(i, text);
		} else {
			QFileInfo fi(e->objectName());
			QString text = fi.fileName();
			if (e->document()->isModified())
				text += " *";
			inputTabs_->setTabText(i, text);
			inputTabs_->setTabToolTip(i, QDir::toNativeSeparators(fi.absoluteFilePath()));
		}
	}
	updateActions();
}

void SqlQueryWidget::start()
{
	outputModel_->setQuery(QSqlQuery());
	if (connectionEdit_->currentIndex() < 0) {
		QMessageBox::critical(this, "", tr("Choose connection"));
		return;
	}

	QPlainTextEdit *e = qobject_cast<QPlainTextEdit *> (inputTabs_->currentWidget());
	if (!e)
		return;

	actionStart_->setEnabled(false);
	actionStop_->setEnabled(true);

	//Remove comments
	QueryThread *thread = new QueryThread(connectionEdit_->currentText(), e->toPlainText(), this);
	connect(thread, SIGNAL(finished()), this, SLOT(queryFinished()));
	connect(actionStop_, SIGNAL(triggered()), thread, SLOT(terminate()));
	thread->start();
	timer_ = startTimer(10);
	time_.start();
}

QStringList SqlQueryWidget::removeComments(const QStringList &sqlQueryes)
{
	static const QRegExp commentRegexp("^\\s*(--)");

	QStringList result;
	std::remove_copy_if(sqlQueryes.begin(), sqlQueryes.end(), std::back_inserter(result),
	[](const QString & line) {
		return commentRegexp.indexIn(line, 0) != -1;
	}
					   );

	return result;
}

QStringList SqlQueryWidget::removeBlankLines(const QStringList &sqlQueryes)
{
	QStringList result;
	std::remove_copy_if(sqlQueryes.begin(), sqlQueryes.end(), std::back_inserter(result),
	[](const QString & line) {
		return line.isEmpty();
	}
					   );

	return result;
}

void SqlQueryWidget::queryFinished()
{
	killTimer(timer_);
	actionStart_->setEnabled(true);
	actionStop_->setEnabled(false);

	QueryThread *thread = qobject_cast <QueryThread *> (sender());
	if (!thread)
		return;

	const QSqlQuery &query = thread->lastQuery();
	thread->deleteLater();

	outputModel_->setQuery(query);

	if (query.lastError().isValid()) {
		messagesEdit_->setPlainText(query.lastError().text());
		outputTabs_->setCurrentWidget(messagesEdit_);
	} else {
		messagesEdit_->setPlainText(tr("The query is successfully comlete for %1 secs").arg(time_.elapsed() / 100));
		if (outputModel_->rowCount() > 0) {
			outputTabs_->setCurrentWidget(outputTable_);
		} else {
			outputTabs_->setCurrentWidget(messagesEdit_);
		}
	}
}

void SqlQueryWidget::connectionsChanged()
{
	const QString &currentConnection = connectionEdit_->currentText();
	connectionEdit_->clear();
	connectionEdit_->addItems(QSqlDatabase::connectionNames());
	connectionEdit_->setCurrentIndex(connectionEdit_->findText(currentConnection, Qt::MatchFixedString));
}

void SqlQueryWidget::updateActions()
{
	QPlainTextEdit *e = qobject_cast<QPlainTextEdit *> (inputTabs_->currentWidget());
	if (!e)
		return;

	actionSave_->setEnabled(e->document()->isModified());
	actionUndo_->setEnabled(e->document()->isUndoAvailable());
	actionRedo_->setEnabled(e->document()->isRedoAvailable());
}

bool SqlQueryWidget::closeTab(int index)
{
	inputTabs_->setCurrentIndex(index);

	QPlainTextEdit *e = qobject_cast<QPlainTextEdit *> (inputTabs_->widget(index));
	if (!e)
		return false;

	if (e->document()->isModified()) {
		int res = QMessageBox::question(this, "", tr("Tab \"%1\" is modified.\nSave?").arg(inputTabs_->tabText(index)),
										QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

		if (res == QMessageBox::Cancel) {
			return false;
		}

		if (res == QMessageBox::Yes) {
			if (!save())
				return false;
		}
	}

	delete e;
	return true;
}

void SqlQueryWidget::undo()
{
	QPlainTextEdit *e = qobject_cast<QPlainTextEdit *> (inputTabs_->currentWidget());
	if (!e)
		return;

	e->undo();
}

void SqlQueryWidget::redo()
{
	QPlainTextEdit *e = qobject_cast<QPlainTextEdit *> (inputTabs_->currentWidget());
	if (!e)
		return;

	e->redo();
}
