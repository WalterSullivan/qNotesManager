/*
This file is part of qNotesManager.

qNotesManager is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

qNotesManager is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with qNotesManager. If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"

#include "navigationpanelwidget.h"
#include "document.h"
#include "application.h"
#include "note.h"
#include "folder.h"
#include "searchwidget.h"
#include "searchresultswidget.h"
#include "documentsearchengine.h"
#include "notestabwidget.h"
#include "documentpropertieswidget.h"
#include "aboutprogramwidget.h"
#include "applicationsettingswidget.h"
#include "cipherer.h"
#include "global.h"
#include "appinfo.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QTimer>
#include <QInputDialog>

using namespace qNotesManager;

MainWindow::MainWindow() : QMainWindow(0) {
	closeDocumentAfterSave = false;
	exitAppAfterSave = false;
	openDocumentAfterSave = false;
	newDocumentAfterSave = false;
	delayedDocumentToOpenFileName = "";

	createControls();

	showToolbarAction->setChecked(Application::I()->Settings.GetShowToolbar());
	toolbar->setVisible(Application::I()->Settings.GetShowToolbar());

	showStatusBarAction->setChecked(Application::I()->Settings.GetShowStausBar());
	statusBar->setVisible(Application::I()->Settings.GetShowStausBar());


	// Clipboard
	QClipboard* clipboard = QApplication::clipboard();
	QObject::connect(clipboard, SIGNAL(dataChanged()),
					 this, SLOT(sl_Clipboard_DataChanged()));
	sl_Clipboard_DataChanged();


	QObject::connect(Application::I(), SIGNAL(sg_CurrentDocumentChanged(Document*)),
					 this, SLOT(sl_Application_CurrentDocumentChanged(Document*)));



	setWindowTitle(VER_PRODUCTNAME_STR);
	updateRecentFilesMenu();

	resize(Application::I()->Settings.GetWindowSize());
	move(Application::I()->Settings.GetWindowPos());
	setWindowState(Application::I()->Settings.GetWindowState());
}

void MainWindow::createActions() {
	newDocumentAction = new QAction(QPixmap(":/gui/document"), "New document", this);
	QObject::connect(newDocumentAction, SIGNAL(triggered()),
					 this, SLOT(sl_NewDocumentAction_Triggered()));

	openDocumentAction = new QAction(QPixmap(":/gui/folder-horizontal-open"), "Open document", this);
	QObject::connect(openDocumentAction, SIGNAL(triggered()),
					 this, SLOT(sl_OpenDocumentAction_Triggered()));
	openDocumentAction->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));

	saveDocumentAction = new QAction(QPixmap(":/gui/disk-black"), "Save document", this);
	QObject::connect(saveDocumentAction, SIGNAL(triggered()),
					 this, SLOT(sl_SaveDocumentAction_Triggered()));
	saveDocumentAction->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
	saveDocumentAction->setEnabled(false);


	saveDocumentAsAction = new QAction(QPixmap("/gui/disk-black"), "Save As...", this);
	QObject::connect(saveDocumentAsAction, SIGNAL(triggered()),
					 this, SLOT(sl_SaveDocumentAsAction_Triggered()));
	saveDocumentAsAction->setEnabled(false);

	closeDocumentAction = new QAction(QPixmap(":/gui/cross"), "Close document", this);
	QObject::connect(closeDocumentAction, SIGNAL(triggered()),
					 this, SLOT(sl_CloseDocumentAction_Triggered()));
	closeDocumentAction->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Q));
	closeDocumentAction->setEnabled(false);

	documentPropertiesAction = new QAction(QPixmap(":/gui/property"), "Document properties", this);
	QObject::connect(documentPropertiesAction, SIGNAL(triggered()),
					 this, SLOT(sl_DocumentPropertiesAction_Triggered()));
	documentPropertiesAction->setEnabled(false);

	globalSearchAction = new QAction(QPixmap(":/gui/magnifier"), "Search", this);
	QObject::connect(globalSearchAction, SIGNAL(triggered()),
					 this, SLOT(sl_GlobalSearchAction_Triggered()));
	globalSearchAction->setShortcut(QKeySequence(Qt::ControlModifier |Qt::ShiftModifier | Qt::Key_F));
	globalSearchAction->setEnabled(false);

	exitAction = new QAction(QPixmap(":/gui/power"), "Exit", this);
	QObject::connect(exitAction, SIGNAL(triggered()),
					 this, SLOT(sl_ExitAction_Triggered()));

	showToolbarAction = new QAction("Show toolbar", this);
	showToolbarAction->setCheckable(true);
	showToolbarAction->setChecked(true);
	QObject::connect(showToolbarAction, SIGNAL(triggered()),
					 this, SLOT(sl_ShowToolbarAction_Triggered()));

	showStatusBarAction = new QAction("Show statusbar", this);
	showStatusBarAction->setCheckable(true);
	showStatusBarAction->setChecked(true);
	QObject::connect(showStatusBarAction, SIGNAL(triggered()),
					 this, SLOT(sl_ShowStatusbarAction_Triggered()));

	applicationSettingAction = new QAction(QPixmap(":/gui/wrench-screwdriver"), "Settings", this);
	QObject::connect(applicationSettingAction, SIGNAL(triggered()),
					 this, SLOT(sl_ApplicationSettingsAction_Triggered()));

	aboutProgramAction = new QAction("About program", this);
	QObject::connect(aboutProgramAction, SIGNAL(triggered()),
					 this, SLOT(sl_AboutProgramAction_Triggered()));

	aboutQtAction = new QAction("About Qt", this);
	QObject::connect(aboutQtAction, SIGNAL(triggered()),
					 this, SLOT(sl_AboutQtAction_Triggered()));

	showHideMainWindowAction = new QAction("Show / hide main window", this);
	QObject::connect(showHideMainWindowAction, SIGNAL(triggered()),
					 this, SLOT(sl_ShowHideMainWindowAction_Triggered()));

	quickNoteAction = new QAction(QIcon(":/gui/lightning"), "Quick note", this);
	quickNoteAction->setToolTip("Create note with text from clipboard");
	quickNoteAction->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_N));
	QObject::connect(quickNoteAction, SIGNAL(triggered()),
					 this, SLOT(sl_QuickNoteAction_Triggered()));
	quickNoteAction->setEnabled(false);
}

void MainWindow::createControls() {
	createActions();

	docProperties = 0;

	navigationPanel = new NavigationPanelWidget();
	QObject::connect(navigationPanel, SIGNAL(sg_NoteDoubleClicked(Note*)),
					 this, SLOT(sl_NoteDoubleClicked(Note*)));
	QObject::connect(navigationPanel, SIGNAL(sg_SelectedItemsActionsListChanged()),
					 this, SLOT(sl_EditMenuContentChanged()));

	notesTabWidget = new NotesTabWidget();
	QObject::connect(notesTabWidget, SIGNAL(sg_CurrentNoteChanged(Note*)),
					 this, SLOT(sl_CurrentNoteChanged(Note*)));

	rightPanelWidget = new QWidget();
	rightPanelSplitter = new QSplitter(Qt::Vertical, this);
	rightPanelSplitter->addWidget(notesTabWidget);
	rightPanelSplitter->setCollapsible(0, false);


	mainSplitter = new QSplitter(Qt::Horizontal, this);
	mainSplitter->addWidget(navigationPanel);
	mainSplitter->addWidget(rightPanelSplitter);
	mainSplitter->setCollapsible(1, false);
	mainSplitter->setStretchFactor(0, 0);
	mainSplitter->setStretchFactor(1, 5);

	setCentralWidget(mainSplitter);

	engine = new DocumentSearchEngine(this);
	searchWidget = 0;
	searchResultsWidget = 0;

	// Create toolbar
	toolbar = new QToolBar(this);
	toolbar->addAction(newDocumentAction);
	toolbar->addAction(openDocumentAction);
	toolbar->addAction(saveDocumentAction);
	toolbar->addAction(closeDocumentAction);
	toolbar->addSeparator();
	toolbar->addAction(documentPropertiesAction);
	toolbar->addAction(globalSearchAction);
	toolbar->addSeparator();
	toolbar->addAction(applicationSettingAction);
	toolbar->addAction(quickNoteAction);
	toolbar->addAction(exitAction);


	addToolBar(toolbar);


	// Create statusbar
	statusBar = new QStatusBar();
	setStatusBar(statusBar);
	statusBarActionLabel = new QLabel();
	statusBar->addWidget(statusBarActionLabel, 3);

	statusBarProgress = new QProgressBar();
	statusBarProgress->setTextVisible(false);
	statusBarProgress->setMinimum(0);
	statusBarProgress->setMaximum(0);
	statusBar->addWidget(statusBarProgress, 1);
	statusBarProgress->setVisible(false);


	// Create menubar
	menuBar = new QMenuBar();
	setMenuBar(menuBar);

	// Create main menu
	documentMenu = new QMenu("Document");
	menuBar->addMenu(documentMenu);

	recentFilesMenu = new QMenu("Recent files");

	documentMenu->addAction(newDocumentAction);
	documentMenu->addAction(openDocumentAction);
	documentMenu->addAction(saveDocumentAction);
	documentMenu->addAction(saveDocumentAsAction);
	documentMenu->addAction(closeDocumentAction);
	documentMenu->addSeparator();
	documentMenu->addAction(documentPropertiesAction);
	documentMenu->addSeparator();
	documentMenu->addAction(globalSearchAction);
	documentMenu->addSeparator();
	documentMenu->addMenu(recentFilesMenu);
	documentMenu->addSeparator();
	documentMenu->addAction(exitAction);

	editMenu = new QMenu("Edit", this);
	editMenu->menuAction()->setEnabled(false);
	menuBar->addMenu(editMenu);

	formatMenu = new QMenu("Format", this);
	formatMenu->menuAction()->setEnabled(false);
	menuBar->addMenu(formatMenu);

	optionsMenu = new QMenu("Options", this);
	menuBar->addMenu(optionsMenu);


	optionsMenu->addAction(showToolbarAction);
	optionsMenu->addAction(showStatusBarAction);
	optionsMenu->addSeparator();
	optionsMenu->addAction(applicationSettingAction);

	aboutMenu = new QMenu("About", this);
	menuBar->addMenu(aboutMenu);



	aboutMenu->addAction(aboutProgramAction);
	aboutMenu->addAction(aboutQtAction);

	// Create tray icon

	addAction(quickNoteAction);

	trayIconMenu = new QMenu();
	trayIconMenu->addAction(showHideMainWindowAction);
	trayIconMenu->addAction(quickNoteAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(exitAction);

	trayIcon = new QSystemTrayIcon(QIcon(":/main"), this);
	QObject::connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
					 this, SLOT(sl_Tray_Activated(QSystemTrayIcon::ActivationReason)));
	trayIcon->setContextMenu(trayIconMenu);
	if (Application::I()->Settings.GetShowSystemTray()) {
		trayIcon->show();
	}

}

/*virtual*/
bool MainWindow::eventFilter (QObject* watched, QEvent* event) {
	return false;
}

/*virtal*/
void MainWindow::closeEvent (QCloseEvent* event) {
	event->ignore();

	if (Application::I()->Settings.GetCloseToTray()) {
		hide();
	} else {
		sl_ExitAction_Triggered();
	}
}

/*virtual*/
void MainWindow::changeEvent (QEvent* event) {
	QMainWindow::changeEvent(event);

	if (event->type() == QEvent::WindowStateChange) {
		if (isMinimized() &&
			(Application::I()->Settings.GetMinimizeToTray())) {
			QTimer::singleShot(0, this, SLOT(hide()));
		}
	}
}

void MainWindow::resizeEvent (QResizeEvent* event) {
	QMainWindow::resizeEvent(event);

	if (!isMaximized()) {Application::I()->Settings.SetWindowSize(size());}
}

void MainWindow::moveEvent (QMoveEvent* event) {
	QMainWindow::moveEvent(event);

	if (!isMaximized()) {Application::I()->Settings.SetWindowPos(pos());}
}

void MainWindow::sl_NoteDoubleClicked(Note* note) {
	if (!note) {
		WARNING("Null pointer recieved");
		return;
	}
	notesTabWidget->OpenNote(note);
}

void MainWindow::sl_ShowSearchResult(NoteFragment fragment) {
	notesTabWidget->ShowFragment(fragment);
}

void MainWindow::sl_NewDocumentAction_Triggered() {
	Document* doc = Application::I()->CurrentDocument();
	if (doc != 0) {
		bool cancelled = false;
		bool delayed = false;

		sl_CloseDocumentAction_Triggered(&cancelled, &delayed);

		if (cancelled) {return;}
		if (delayed) {newDocumentAfterSave = true; return;}
	}

	Document* newDoc = new Document();
	Application::I()->SetCurrentDocument(newDoc);
	setWindowModified(true);
}

void MainWindow::sl_OpenDocumentAction_Triggered() {
	QString fileName = QString();

	while(fileName.isEmpty()) {
		fileName = QFileDialog::getOpenFileName(this, "Select a file to load", QString(),
												"qNotesManager save file (*.nms)");
		if (fileName.isNull()) {return;}

		QFile file(fileName);
		if (fileName.isEmpty() || !file.exists()) {
			QMessageBox::warning(this, QString(), "Select file to open");
		} else {
			break;
		}
	}


	Document* doc = Application::I()->CurrentDocument();
	if (doc != 0) {
		bool cancelled = false;
		bool delayed = false;

		sl_CloseDocumentAction_Triggered(&cancelled, &delayed);

		if (cancelled) {return;}
		if (delayed) {
			openDocumentAfterSave = true;
			delayedDocumentToOpenFileName = fileName;
			return;
		}
	}


	OpenDocument(fileName);
}

void MainWindow::OpenDocument(QString fileName) {
	tempDocument = new Document();

	QObject::connect(tempDocument, SIGNAL(sg_LoadingAborted()), this, SLOT(sl_Document_LoadingAborted()));
	QObject::connect(tempDocument, SIGNAL(sg_LoadingFailed(QString)), this, SLOT(sl_Document_LoadingFailed(QString)));
	QObject::connect(tempDocument, SIGNAL(sg_LoadingFinished()), this, SLOT(sl_Document_LoadingFinished()));
	QObject::connect(tempDocument, SIGNAL(sg_LoadingPartiallyFinished()), this, SLOT(sl_Document_LoadingPartiallyFinished()));
	QObject::connect(tempDocument, SIGNAL(sg_LoadingProgress(int)), this, SLOT(sl_Document_LoadingProgress(int)));
	QObject::connect(tempDocument, SIGNAL(sg_LoadingStarted()), this, SLOT(sl_Document_LoadingStarted()));
	QObject::connect(tempDocument, SIGNAL(sg_ConfirmationRequest(QSemaphore*,QString,bool*)), this, SLOT(sl_Document_ConfirmationRequest(QSemaphore*,QString,bool*)));
	QObject::connect(tempDocument, SIGNAL(sg_Message(QString)), this, SLOT(sl_Document_Message(QString)));
	QObject::connect(tempDocument, SIGNAL(sg_PasswordRequired(QSemaphore*,QString*,bool)), this, SLOT(sl_Document_PasswordRequired(QSemaphore*,QString*,bool)));


	tempDocument->Open(fileName);
	newRecentFile(fileName);
}

void MainWindow::sl_SaveDocumentAction_Triggered(bool* actionCancelled) {
	Document* doc = Application::I()->CurrentDocument();
	if (!doc) {
		WARNING("No current document set");
		return;
	}

	QString filename = QString();
	if (doc->GetFilename().isEmpty()) {
		filename = QFileDialog::getSaveFileName(this, "Select a name", QString(),
												"qNotesManager save file (*.nms)");
		if (filename.isNull() || filename.isEmpty()) {
			if (actionCancelled) {*actionCancelled = true;}
			return;
		}
		newRecentFile(filename);
	}

	doc->Save(filename);
}

void MainWindow::sl_SaveDocumentAsAction_Triggered() {
	Document* doc = Application::I()->CurrentDocument();
	if (doc == 0) {
		WARNING("No current document set");
		return;
	}

	QString filename = QFileDialog::getSaveFileName(this, "Select a name", QString(),
		"qNotesManager save file (*.nms)");
	if (filename.isNull()) {return;}

	newRecentFile(filename);
	doc->Save(filename);
}

void MainWindow::sl_CloseDocumentAction_Triggered(bool* actionCancelled, bool* actionDelayed) {
	Document* oldDoc = Application::I()->CurrentDocument();
	if (oldDoc == 0) {
		WARNING("No current document set");
		return;
	}

	if (oldDoc->HasUnsavedData()) {
		QMessageBox::StandardButton result = QMessageBox::question(this, "Save document?",
											"Current document has been changed. Save it?",
											QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
											QMessageBox::Yes);
		if (result == QMessageBox::Cancel) {
			if (actionCancelled) {*actionCancelled = true;}
			return;
		}
		if (result == QMessageBox::Yes) {
			bool saveCancelled = false;
			sl_SaveDocumentAction_Triggered(&saveCancelled);
			if (saveCancelled) {
				if (actionCancelled) {*actionCancelled = saveCancelled;}
				return;
			}

			closeDocumentAfterSave = true;
			if (actionDelayed) {
				*actionDelayed = true;
			}
			return;
		}
	}

	Application::I()->SetCurrentDocument(0);
	notesTabWidget->Clear();
	delete oldDoc;
}

void MainWindow::sl_DocumentPropertiesAction_Triggered() {
	Document* doc = Application::I()->CurrentDocument();
	if (!doc) {
		WARNING("Current document not set");
		return;
	}

	if (!docProperties) {
		docProperties = new DocumentPropertiesWidget(this);
	}

	docProperties->SetDocument(doc);
	docProperties->exec();
}

void MainWindow::sl_GlobalSearchAction_Triggered() {
	if (!searchWidget) {
		searchWidget = new SearchWidget(engine);
	}
	if (!searchResultsWidget) {
		searchResultsWidget = new SearchResultsWidget(engine);
		QObject::connect(searchResultsWidget, SIGNAL(sg_ShowSearchResults(NoteFragment)),
						 this, SLOT(sl_ShowSearchResult(NoteFragment)));
		QObject::connect(searchResultsWidget, SIGNAL(sg_CloseRequest()),
						 this, SLOT(sl_SearchResults_CloseRequest()));
		QObject::connect(searchResultsWidget, SIGNAL(sg_ShowRequest()),
						 this, SLOT(sl_SearchResults_ShowRequest()));
		searchResultsWidget->hide();
		rightPanelSplitter->addWidget(searchResultsWidget);
	}
	searchWidget->show();
}

void MainWindow::sl_SearchResults_CloseRequest() {
	searchResultsWidget->hide();
}

void MainWindow::sl_SearchResults_ShowRequest() {
	searchResultsWidget->show();
}

void MainWindow::sl_ExitAction_Triggered() {
	Document* doc = Application::I()->CurrentDocument();

	if (doc != 0) {
		bool cancelled = false;
		bool delayed = false;

		sl_CloseDocumentAction_Triggered(&cancelled, &delayed);

		if (cancelled) {return;}
		if (delayed) {exitAppAfterSave = true; return;}
	}

	QCoreApplication::quit();
}

void MainWindow::sl_ShowToolbarAction_Triggered() {
	Application::I()->Settings.SetShowToolbar(showToolbarAction->isChecked());
	toolbar->setVisible(showToolbarAction->isChecked());
}

void MainWindow::sl_ShowStatusbarAction_Triggered() {
	Application::I()->Settings.SetShowToolbar(showToolbarAction->isChecked());
	statusBar->setVisible(showStatusBarAction->isChecked());
}

void MainWindow::sl_ApplicationSettingsAction_Triggered() {
	ApplicationSettingsWidget w;
	if (w.exec() == QDialog::Accepted) {
		trayIcon->setVisible(Application::I()->Settings.GetShowSystemTray());
		navigationPanel->UpdateViewsVisibility();
	}
}

void MainWindow::sl_AboutProgramAction_Triggered() {
	AboutProgramWidget w;
	w.exec();
}

void MainWindow::sl_AboutQtAction_Triggered() {
	QMessageBox::aboutQt(this);
}

void MainWindow::sl_CurrentDocument_Changed() {
	updateWindowTitle();
}

void MainWindow::sl_Application_CurrentDocumentChanged(Document* oldDoc) {
	if (oldDoc) {
		QObject::disconnect(oldDoc, 0, this, 0);
	}

	Document* doc = Application::I()->CurrentDocument();
	engine->SetTargetDocument(doc);
	navigationPanel->SetTargetDocument(doc);
	if (doc) {
		QObject::connect(doc, SIGNAL(sg_Changed()),
						 this, SLOT(sl_CurrentDocument_Changed()));
		QObject::connect(doc, SIGNAL(sg_ItemUnregistered(Note*)),
						 this, SLOT(sl_Application_NoteDeleted(Note*)));

		QObject::connect(doc, SIGNAL(sg_SavingAborted()),
						 this, SLOT(sl_Document_SavingAborted()));
		QObject::connect(doc, SIGNAL(sg_SavingFailed(QString)),
						 this, SLOT(sl_Document_SavingFailed(QString)));
		QObject::connect(doc, SIGNAL(sg_SavingFinished()),
						 this, SLOT(sl_Document_SavingFinished()));
		QObject::connect(doc, SIGNAL(sg_SavingProgress(int)),
						 this, SLOT(sl_Document_SavingFinished()));
		QObject::connect(doc, SIGNAL(sg_SavingStarted()),
						 this, SLOT(sl_Document_SavingStarted()));

		Application::I()->Settings.SetLastDocumentName(doc->GetFilename());
	}

	sl_EditMenuContentChanged();

	// Actions
	bool enable = (doc != 0);
	saveDocumentAction->setEnabled(enable);
	saveDocumentAsAction->setEnabled(enable);
	closeDocumentAction->setEnabled(enable);
	documentPropertiesAction->setEnabled(enable);
	globalSearchAction->setEnabled(enable);
	sl_Clipboard_DataChanged();

	updateWindowTitle();
}

void MainWindow::sl_OpenRecentFileAction_Triggered() {
	// Find sender action
	QObject* sender = QObject::sender();
	if (sender == 0) {return;}

	QAction* senderAction = 0;
	senderAction = dynamic_cast<QAction*>(sender);
	if (senderAction == 0) {return;}

	QString fileName = senderAction->text();

	// Chech if file exists
	QFileInfo fileInfo(fileName);
	if (!fileInfo.exists()) {
		QMessageBox::StandardButton result = QMessageBox::question(this, "File not found",
											"Selected file not found. Delete it from history?",
											QMessageBox::Yes | QMessageBox::No,
											QMessageBox::Yes);
		if (result == QMessageBox::No) {
			return;
		}
		if (result == QMessageBox::Yes) {
			QStringList recentFilesList = Application::I()->Settings.GetRecentFiles();
			recentFilesList.removeAll(fileName);
			Application::I()->Settings.SetRecentFiles(recentFilesList);
			updateRecentFilesMenu();
			return;
		}
	}

	Document* doc = Application::I()->CurrentDocument();
	if (doc != 0) {
		bool cancelled = false;
		bool delayed = false;

		sl_CloseDocumentAction_Triggered(&cancelled, &delayed);

		if (cancelled) {return;}
		if (delayed) {
			openDocumentAfterSave = true;
			delayedDocumentToOpenFileName = fileName;
			return;
		}
	}

	OpenDocument(fileName);
}

void MainWindow::updateWindowTitle() {
	Document* doc = Application::I()->CurrentDocument();
	if (doc == 0) {
		setWindowTitle(VER_PRODUCTNAME_STR);
		setWindowModified(false);
	} else {
		QString title;
		title = doc->GetFilename().isEmpty() ? "<unsaved>" : doc->GetFilename();
		title.append("[*] - ").append(VER_PRODUCTNAME_STR);
		setWindowModified(doc->HasUnsavedData());
		setWindowTitle(title);
	}
}

void MainWindow::updateRecentFilesMenu() {
	recentFilesMenu->clear();
	QStringList recentFilesList = Application::I()->Settings.GetRecentFiles();

	foreach (QString file, recentFilesList) {
		QAction* action = new QAction(file, recentFilesMenu);
		QObject::connect(action, SIGNAL(triggered()),
						 this, SLOT(sl_OpenRecentFileAction_Triggered()));
		recentFilesMenu->addAction(action);
	}

	recentFilesMenu->setEnabled((recentFilesMenu->actions().count() != 0));
}

void MainWindow::newRecentFile(const QString& fileName) {
	const int maxRecentFiles = 6;

	QStringList recentFilesList = Application::I()->Settings.GetRecentFiles();
	if (recentFilesList.contains(fileName)) {
		recentFilesList.removeAll(fileName);
	}
	while (recentFilesList.count() >= maxRecentFiles) {
		recentFilesList.removeLast();
	}

	recentFilesList.insert(0, fileName);

	Application::I()->Settings.SetRecentFiles(recentFilesList);

	updateRecentFilesMenu();
}

void MainWindow::sl_Application_NoteDeleted(Note* n) {
	if (notesTabWidget->DisplayedNotes().contains(n)) {
		notesTabWidget->CloseNote(n);
	}
}

void MainWindow::sl_CurrentNoteChanged(Note* note) {
	if (formatMenu->actions().size() > 0) {
		formatMenu->clear();
	}

	const QList<QAction*> actions = notesTabWidget->GetCurrentEditActionsList();

	if (actions.isEmpty()) {
		formatMenu->menuAction()->setEnabled(false);
	} else {
		formatMenu->menuAction()->setEnabled(true);
		foreach (QAction* action, actions) {
			formatMenu->addAction(action);
		}
	}
}

void MainWindow::sl_ShowHideMainWindowAction_Triggered() {
	if (this->isVisible()) {
		this->hide();
	} else {
		this->show();
		setWindowState(windowState() & ~Qt::WindowMinimized); // if window was hidden by minimizing
		this->activateWindow();
	}
}

void MainWindow::sl_Tray_Activated (QSystemTrayIcon::ActivationReason reason) {
	if (reason == QSystemTrayIcon::Trigger) {
		sl_ShowHideMainWindowAction_Triggered();
	}
}

void MainWindow::sl_Clipboard_DataChanged() {
	if (Application::I()->CurrentDocument() == 0) {
		quickNoteAction->setEnabled(false);
		return;
	}
	const QMimeData* data = QApplication::clipboard()->mimeData();
	quickNoteAction->setEnabled(data->hasText() || data->hasHtml());
}

void MainWindow::sl_QuickNoteAction_Triggered() {
	const int maxCaptionSize = 16;

	Document* doc = Application::I()->CurrentDocument();
	if (doc == 0) {
		WARNING("Current document not set");
		return;
	}

	const QMimeData* data = QApplication::clipboard()->mimeData();
	if (!(data->hasText() || data->hasHtml())) {return;}

	Note* n = new Note("");
	QString caption = data->text().mid(0, maxCaptionSize) + "...";
	caption.replace(QRegExp("[\a\e\f\n\r\t\v]"), " ");
	n->SetName(caption);

	if (data->hasHtml()) {
		n->SetHtml(data->html());
	} else if (data->hasText()) {
		n->SetText(data->text());
	}

	doc->GetTempFolder()->Items.Add(n);
}

void MainWindow::sl_EditMenuContentChanged() {
	if (!Application::I()->CurrentDocument()) {
		editMenu->menuAction()->setEnabled(false);
		return;
	}

	if (editMenu->actions().size() > 0) {
		editMenu->clear();
	}

	const QList<QAction*> actions = navigationPanel->GetSelectedItemsActions();

	if (actions.isEmpty()) {
		editMenu->menuAction()->setEnabled(false);
	} else {
		editMenu->menuAction()->setEnabled(true);
		foreach (QAction* action, actions) {
			editMenu->addAction(action);
		}
	}
}

void MainWindow::sl_QApplication_AboutToQuit() {
	Application::I()->Settings.SetWindowState(windowState());
}

void MainWindow::sl_Document_LoadingStarted() {
	statusBarActionLabel->setText("Loading document...");
	statusBarProgress->setVisible(true);
	statusBarProgress->setValue(0);

	mainSplitter->setEnabled(false);
	toolbar->setEnabled(false);
	menuBar->setEnabled(false);
}

void MainWindow::sl_Document_LoadingProgress(int progress) {
	statusBarProgress->setValue(progress);
}

void MainWindow::sl_Document_LoadingPartiallyFinished() {

}

void MainWindow::sl_Document_LoadingFinished() {
	statusBarActionLabel->setText("");
	statusBarProgress->reset();
	statusBarProgress->setVisible(false);
	Application::I()->SetCurrentDocument(tempDocument);
	tempDocument = 0;

	mainSplitter->setEnabled(true);
	toolbar->setEnabled(true);
	menuBar->setEnabled(true);
}

void MainWindow::sl_Document_PasswordRequired(QSemaphore* s, QString* p, bool lastTryFailed) {
	bool ok = false;

	QString password;
	QString message = lastTryFailed ? "Incorrect. Try again" :
					  "This document is protected. Enter password";

	while (password.isEmpty()) {
		password = QInputDialog::getText(0, "Enter password",
										 message,
										 QLineEdit::Password, "", &ok);
		if (!ok) {
			break;
		}
	}

	*p = password;
	s->release();
}

void MainWindow::sl_Document_ConfirmationRequest(QSemaphore* s, QString str, bool* abort) {
	QMessageBox::StandardButton result =
				QMessageBox::question(0, "Warning", str, QMessageBox::Yes | QMessageBox::No);

	if (result == QMessageBox::No) {
		*abort = true;
	}
	s->release();
}

void MainWindow::sl_Document_Message(QString str) {
	QMessageBox::information(0, "", str);
}

void MainWindow::sl_Document_LoadingFailed(QString errorString) {
	QMessageBox::critical(0, "", errorString);

	sl_Document_LoadingAborted();
}

void MainWindow::sl_Document_LoadingAborted() {
	tempDocument->deleteLater();
	tempDocument = 0;
	statusBarActionLabel->setText("");
	statusBarProgress->reset();
	statusBarProgress->setVisible(false);

	mainSplitter->setEnabled(true);
	toolbar->setEnabled(true);
	menuBar->setEnabled(true);
}

void MainWindow::sl_Document_SavingStarted() {
	statusBarActionLabel->setText("Saving document...");
	statusBarProgress->setVisible(true);
	statusBarProgress->setValue(0);

	mainSplitter->setEnabled(false);
	toolbar->setEnabled(false);
	menuBar->setEnabled(false);
}

void MainWindow::sl_Document_SavingProgress(int progress) {
	statusBarProgress->setValue(progress);
}

void MainWindow::sl_Document_SavingFinished() {
	statusBarActionLabel->setText("");
	statusBarProgress->reset();
	statusBarProgress->setVisible(false);

	mainSplitter->setEnabled(true);
	toolbar->setEnabled(true);
	menuBar->setEnabled(true);

	setWindowModified(false);
	updateWindowTitle();

	if (closeDocumentAfterSave) {
		closeDocumentAfterSave = false;
		sl_CloseDocumentAction_Triggered();
	}

	if (openDocumentAfterSave) {
		openDocumentAfterSave = false;
		OpenDocument(delayedDocumentToOpenFileName);
		delayedDocumentToOpenFileName = "";
	}

	if (newDocumentAfterSave) {
		newDocumentAfterSave = false;
		sl_NewDocumentAction_Triggered();
	}

	if (exitAppAfterSave) {
		exitAppAfterSave = false;
		sl_ExitAction_Triggered();
	}
}

void MainWindow::sl_Document_SavingFailed(QString errorString) {
	QMessageBox::critical(0, "", errorString);

	sl_Document_SavingAborted();
}

void MainWindow::sl_Document_SavingAborted() {
	statusBarActionLabel->setText("");
	statusBarProgress->reset();
	statusBarProgress->setVisible(false);

	mainSplitter->setEnabled(true);
	toolbar->setEnabled(true);
	menuBar->setEnabled(true);

	closeDocumentAfterSave = false;
	exitAppAfterSave = false;
	openDocumentAfterSave = false;
	newDocumentAfterSave = false;
	delayedDocumentToOpenFileName = "";
}
