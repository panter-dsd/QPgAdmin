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

#ifndef SQLQUERYWIDGET_H
#define SQLQUERYWIDGET_H

class QTabWidget;
class QPlainTextEdit;
class QTableView;
class QToolBar;
class QAction;
class QSplitter;
class QComboBox;
class QSqlQueryModel;
class QStatusBar;

#include <QtCore/QTime>

#include <QtGui/QWidget>

class SqlQueryWidget : public QWidget {
	Q_OBJECT

private:
	QString m_connectionName;
	QTime m_time;
	int m_timer;

	QTabWidget *inputTabs;
	QTabWidget *outputTabs;
	QList<QPlainTextEdit*> sqlEdits;
	QPlainTextEdit *messagesEdit;
	QTableView *outputTable;
	QSqlQueryModel *outputModel;
	QToolBar *toolBar;
	QSplitter *splitter;
	QComboBox *connectionEdit;
	QStatusBar *statusBar;

	QAction *actionAddSqlEditor;
	QAction *actionOpen;
	QAction *actionSave;
	QAction *actionSaveAs;
	QAction *actionStart;
	QAction *actionStop;
	QAction *actionUndo;
	QAction *actionRedo;

public:
	SqlQueryWidget (const QString& connectionName = "", QWidget *parent = 0);
	~SqlQueryWidget ();

private:
	void loadSettings();
	void saveSettings();
	void retranslateStrings ();

protected:
	bool event(QEvent *ev);

private Q_SLOTS:
	void updateTabCaptions ();
	QPlainTextEdit* addSqlEditor ();
	void open ();
	bool save ();
	bool saveAs ();
	void start ();
	void queryFinished ();
	void undo ();
	void redo ();

public Q_SLOTS:
	void connectionsChanged ();
	void updateActions ();
	bool closeTab (int index);
};

#endif //SQLQUERYWIDGET_H
