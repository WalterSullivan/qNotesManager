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
#include "operationabortedexception.h"
#include "ioexception.h"

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

using namespace qNotesManager;

MainWindow::MainWindow() : QMainWindow(0) {
	createControls();

	showToolbarAction->setChecked(Application::I()->Settings.showToolbar);
	toolbar->setVisible(Application::I()->Settings.showToolbar);

	showStatusBarAction->setChecked(Application::I()->Settings.showStausBar);
	statusBar->setVisible(Application::I()->Settings.showStausBar);


	// Clipboard
	QClipboard* clipboard = QApplication::clipboard();
	QObject::connect(clipboard, SIGNAL(dataChanged()),
					 this, SLOT(sl_Clipboard_DataChanged()));
	sl_Clipboard_DataChanged();


	QObject::connect(Application::I(), SIGNAL(sg_CurrentDocumentChanged(Document*)),
					 this, SLOT(sl_Application_CurrentDocumentChanged(Document*)));



	setWindowTitle(APPNAME);

	resize(Application::I()->Settings.windowSize);
	move(Application::I()->Settings.windowPosition);
	setWindowState(Application::I()->Settings.windowState);
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


	// Create menubar
	menuBar = new QMenuBar();
	setMenuBar(menuBar);

	// Create main menu
	documentMenu = new QMenu("Document");
	menuBar->addMenu(documentMenu);

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
	documentMenu->addAction(exitAction);

	editMenu = new QMenu("Edit", this);

	formatMenu = new QMenu("Format", this);


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
	if (Application::I()->Settings.showSystemTray) {
		trayIcon->show();
	}

}

/*virtual*/
bool MainWindow::eventFilter (QObject* watched, QEvent* event) {
	return false;
}

/*virtal*/
void MainWindow::closeEvent (QCloseEvent* event) {
	if (Application::I()->Settings.closeToTray) {
		hide();
	} else {
		Document* doc = Application::I()->CurrentDocument();
		if (doc != 0) {
			bool cancelled = false;
			sl_CloseDocumentAction_Triggered(&cancelled);
			if (cancelled) {return;}
		}

		QCoreApplication::quit();
	}
	event->accept();
}

/*virtual*/
void MainWindow::changeEvent (QEvent* event) {
	QMainWindow::changeEvent(event);
	if ( (event->type() == QEvent::WindowStateChange) &&
			isMinimized() &&
			(Application::I()->Settings.minimizeToTray)) {
		QTimer::singleShot(0, this, SLOT(hide()));
	}
}

void MainWindow::sl_NoteDoubleClicked(Note* note) {
	Q_ASSERT(note != 0);
	notesTabWidget->OpenNote(note);
}

void MainWindow::sl_ShowSearchResult(NoteFragment fragment) {
	notesTabWidget->ShowFragment(fragment);
}

void MainWindow::sl_NewDocumentAction_Triggered() {
	Document* doc = Application::I()->CurrentDocument();
	if (doc != 0) {
		bool cancelled = false;
		sl_CloseDocumentAction_Triggered(&cancelled);
		if (cancelled) {return;}
	}

	Document* newDoc = new Document();
	Application::I()->SetCurrentDocument(newDoc);
	setWindowModified(true);
}

void MainWindow::sl_OpenDocumentAction_Triggered() {

	QString fileName = QString();

	while(fileName.isEmpty()) {
		fileName = QFileDialog::getOpenFileName(this, "Select a file to load");
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
		sl_CloseDocumentAction_Triggered(&cancelled);
		if (cancelled) {return;}
	}

	Document* newDoc = 0;
	try {
		newDoc = Document::Open(fileName);
	} catch (const QCAException& e) {
		QMessageBox::warning(this, QString(),
							 "Encryption error. Make sure all required dlls are present.");
		return;
	} catch (const WrongFileException& e) {
		QMessageBox::information(this, QString(),
			QString().append("This file doesn't seem to be ").append(APPNAME).append(" save file"));
		return;
	} catch (const WrongFileVersionException& e) {
		QMessageBox::information(this, QString(), "This file was created in older version of the "
								 "program and can't be loaded");
		return;
	} catch (const OperationAbortedException& e) {
		return;
	} catch (const IOException& e) {
		QMessageBox::warning(this, QString(), "Ploblem loading file");
		return;
	}

	Q_ASSERT(newDoc != 0);

	if (newDoc == 0) {
		QMessageBox::critical(this, "", "Error loading document");
		return;
	}
	Application::I()->SetCurrentDocument(newDoc);
}

void MainWindow::sl_SaveDocumentAction_Triggered(bool* actionCancelled) {
	Document* doc = Application::I()->CurrentDocument();
	if (doc == 0) {return;}

	saveDocumentVisualSettings();

	QString filename = QString();
	if (doc->GetFilename().isEmpty()) {
		filename = QFileDialog::getSaveFileName(this, "Select a name");
		if (filename.isNull() || filename.isEmpty()) {
			if (actionCancelled != 0) {*actionCancelled = true;}
			return;
		}

	}

	try {
		doc->Save(filename);
	} catch (const QCAException& e) {
		QMessageBox::warning(this, QString(), "Encryption error. Make sure all required dlls are present.");
		return;
	} catch (const IOException& e) {
		QMessageBox::warning(this, QString(), "Ploblem loading file");
		return;
	} catch (Exception& e) {
		QMessageBox::warning(this, QString(), "Ploblem loading file");
		return;
	}


	setWindowModified(false);
}

void MainWindow::sl_SaveDocumentAsAction_Triggered() {
	Document* doc = Application::I()->CurrentDocument();
	if (doc == 0) {return;}

	saveDocumentVisualSettings();
	QString filename = QFileDialog::getSaveFileName(this, "Select a name");
	if (filename.isNull()) {return;}

	try {
		doc->Save(filename);
	} catch (const QCAException& e) {
		QMessageBox::warning(this, QString(), "Encryption error. Make sure all required dlls are present.");
		return;
	} catch (const IOException& e) {
		QMessageBox::warning(this, QString(), "Ploblem loading file");
		return;
	} catch (Exception& e) {
		QMessageBox::warning(this, QString(), "Ploblem loading file");
		return;
	}

	setWindowModified(false);
}

void MainWindow::sl_CloseDocumentAction_Triggered(bool* actionCancelled) {
	Document* oldDoc = Application::I()->CurrentDocument();
	if (oldDoc == 0) {return;}

	if (oldDoc->IsChanged()) {
		QMessageBox::StandardButton result = QMessageBox::question(this, "Save document?",
											"Current document has been changed. Save it?",
											QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
											QMessageBox::Yes);
		if (result == QMessageBox::Cancel) {
			if (actionCancelled != 0) {*actionCancelled = true;}
			return;
		}
		if (result == QMessageBox::Yes) {
			bool saveCancelled = false;
			sl_SaveDocumentAction_Triggered(&saveCancelled);
			if (saveCancelled) {
				if (actionCancelled != 0) {*actionCancelled = saveCancelled;}
				return;
			}
		}
	}

	Application::I()->SetCurrentDocument(0);
	notesTabWidget->Clear();
	delete oldDoc;
}

void MainWindow::sl_DocumentPropertiesAction_Triggered() {
	Q_ASSERT(Application::I()->CurrentDocument() != 0);

	if (docProperties == 0) {
		docProperties = new DocumentPropertiesWidget(0);
	}

	docProperties->SetDocument(Application::I()->CurrentDocument());
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
		sl_CloseDocumentAction_Triggered(&cancelled);
		if (cancelled) {return;}
	}

	QCoreApplication::quit();
}

void MainWindow::sl_ShowToolbarAction_Triggered() {
	Application::I()->Settings.showToolbar = showToolbarAction->isChecked();
	toolbar->setVisible(showToolbarAction->isChecked());
}

void MainWindow::sl_ShowStatusbarAction_Triggered() {
	Application::I()->Settings.showToolbar = showToolbarAction->isChecked();
	statusBar->setVisible(showStatusBarAction->isChecked());
}

void MainWindow::sl_ApplicationSettingsAction_Triggered() {
	ApplicationSettingsWidget w;
	if (w.exec() == QDialog::Accepted) {
		trayIcon->setVisible(Application::I()->Settings.showSystemTray);
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
	setWindowModified(Application::I()->CurrentDocument()->IsChanged());
}

void MainWindow::sl_Application_CurrentDocumentChanged(Document* oldDoc) {
	if (oldDoc) {
		QObject::disconnect(oldDoc, 0, this, 0);
	}

	Document* doc = Application::I()->CurrentDocument();
	engine->SetTargetDocument(doc);
	navigationPanel->SetTargetDocument(doc);
	if (doc == 0) {
		setWindowTitle(APPNAME);
		setWindowModified(false);
	} else {
		QObject::connect(doc, SIGNAL(sg_Changed()),
						 this, SLOT(sl_CurrentDocument_Changed()));
		QObject::connect(doc, SIGNAL(sg_ItemUnregistered(Note*)),
						 this, SLOT(sl_Application_NoteDeleted(Note*)));
		QString title;
		title = doc->GetFilename().isEmpty() ? "<unsaved>" : doc->GetFilename();
		title.append("[*] - ").append(APPNAME);
		setWindowModified(doc->IsChanged());
		setWindowTitle(title);

		restoreDocumentVisualSettings();
		sl_Clipboard_DataChanged();
	}

	// Actions
	bool enable = (doc != 0);
	saveDocumentAction->setEnabled(enable);
	saveDocumentAsAction->setEnabled(enable);
	closeDocumentAction->setEnabled(enable);
	documentPropertiesAction->setEnabled(enable);
	globalSearchAction->setEnabled(enable);





}

void MainWindow::sl_Application_NoteDeleted(Note* n) {
	if (notesTabWidget->DisplayedNotes().contains(n)) {
		notesTabWidget->CloseNote(n);
	}
}

void MainWindow::saveDocumentVisualSettings() const {
	Document* doc = Application::I()->CurrentDocument();
	if (doc == 0) {return;}

	doc->VisualSettings.ActiveNavigationTab = navigationPanel->CurrentTabIndex();
	doc->VisualSettings.ActiveNote = notesTabWidget->CurrentNote();
	doc->VisualSettings.OpenedNotes = notesTabWidget->GetState();
}

void MainWindow::restoreDocumentVisualSettings() {
	Document* doc = Application::I()->CurrentDocument();
	if (doc == 0) {return;}

	navigationPanel->SetCurrentTab(doc->VisualSettings.ActiveNavigationTab);
	const QList<Note*> notes = doc->GetNotesList();

	QPair<Note*, int> pair;
	foreach (pair , doc->VisualSettings.OpenedNotes) {
		if (notes.contains(pair.first)) {
			notesTabWidget->OpenNote(pair.first, pair.second, true);
		}
	}

	if (notes.contains(doc->VisualSettings.ActiveNote)) {
		notesTabWidget->SetCurrentNote(doc->VisualSettings.ActiveNote);
	}
}

void MainWindow::sl_CurrentNoteChanged(Note* note) {
	if (formatMenu->actions().size() > 0) {
		formatMenu->clear();
	}

	const QList<QAction*> actions = notesTabWidget->GetCurrentEditActionsList();

	if (actions.isEmpty()) {
		if (menuBar->actions().contains(formatMenu->menuAction())) {
			menuBar->removeAction(formatMenu->menuAction());
		}
	} else {
		if (!menuBar->actions().contains(formatMenu->menuAction())) {
			menuBar->insertMenu(optionsMenu->menuAction(), formatMenu);
		}
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
	if (doc == 0) {return;}

	const QMimeData* data = QApplication::clipboard()->mimeData();
	if (!(data->hasText() || data->hasHtml())) {return;}

	Note* n = new Note("");

	if (data->hasHtml()) {
		n->SetName(data->text().mid(0, maxCaptionSize) + "...");
		n->SetHtml(data->html());
	} else if (data->hasText()) {
		n->SetName(data->text().mid(0, maxCaptionSize) + "...");
		n->SetText(data->text());
	}

	doc->GetTempFolder()->Items.Add(n);
}

void MainWindow::sl_EditMenuContentChanged() {
	if (editMenu->actions().size() > 0) {
		editMenu->clear();
	}

	const QList<QAction*> actions = navigationPanel->GetSelectedItemsActions();

	if (actions.isEmpty()) {
		if (menuBar->actions().contains(editMenu->menuAction())) {
			menuBar->removeAction(editMenu->menuAction());
		}
	} else {
		if (!menuBar->actions().contains(editMenu->menuAction())) {
			QAction* before = 0;
			if (menuBar->actions().contains(formatMenu->menuAction())) {
				before = formatMenu->menuAction();
			} else {
				before = optionsMenu->menuAction();
			}
			menuBar->insertMenu(before, editMenu);
		}
		foreach (QAction* action, actions) {
			editMenu->addAction(action);
		}
	}
}

void MainWindow::sl_QApplication_AboutToQuit() {
	Application::I()->Settings.windowPosition = pos();
	Application::I()->Settings.windowSize = size();
	Application::I()->Settings.windowState = windowState();

	Application::I()->Settings.Save();
}
