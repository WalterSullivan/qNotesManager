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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "notefragment.h"

#include <QMainWindow>
#include <QList>
#include <QCheckBox>
#include <QDateTimeEdit>
#include <QLabel>
#include <QTextEdit>
#include <QEvent>
#include <QLayout>
#include <QAction>
#include <QMenu>
#include <QStatusBar>
#include <QMenuBar>
#include <QSplitter>
#include <QSystemTrayIcon>

namespace qNotesManager {
	class NavigationPanelWidget;
	class Note;
	class TagsLineEdit;
	class DocumentSearchEngine;
	class SearchWidget;
	class SearchResultsWidget;
	class NotesTabWidget;
	class DocumentPropertiesWidget;
	class Document;

	class MainWindow : public QMainWindow {
	Q_OBJECT
	private:
		NavigationPanelWidget* navigationPanel;
		NotesTabWidget* notesTabWidget;
		QSplitter* mainSplitter;

		QToolBar* toolbar;
		QStatusBar* statusBar;
		QMenuBar* menuBar;

		QSystemTrayIcon* trayIcon;
		QMenu* trayIconMenu;
		QAction* showHideMainWindowAction;
		QAction* quickNoteAction;


		// Main menu
		QMenu* documentMenu;
		QAction* newDocumentAction;
		QAction* openDocumentAction;
		QAction* saveDocumentAction;
		QAction* saveDocumentAsAction;
		QAction* closeDocumentAction;
		QAction* documentPropertiesAction;
		QAction* globalSearchAction;
		QAction* exitAction;

		QMenu* editMenu;

		QMenu* formatMenu;

		QMenu* optionsMenu;
		QAction* showToolbarAction;
		QAction* showStatusBarAction;
		QAction* applicationSettingAction;

		QMenu* aboutMenu;
		QAction* aboutProgramAction;
		QAction* aboutQtAction;


		DocumentSearchEngine* engine;
		SearchWidget* searchWidget;
		SearchResultsWidget* searchResultsWidget;

		QWidget* rightPanelWidget;
		QSplitter* rightPanelSplitter;

		DocumentPropertiesWidget* docProperties;

		void saveDocumentVisualSettings() const;
		void restoreDocumentVisualSettings();
		void createActions();
		void createControls();

	public:
		explicit MainWindow();
		void OpenDocument(QString fileName);

	protected:
		/*virtual*/ bool eventFilter (QObject* watched, QEvent* event);
		/*virtual*/ void closeEvent (QCloseEvent* event);
		/*virtual*/ void changeEvent (QEvent* event);
	signals:

	public slots:
	private slots:
		void sl_NoteDoubleClicked(Note*);

		void sl_CurrentNoteChanged(Note*);


		void sl_ShowSearchResult(NoteFragment);
		void sl_SearchResults_CloseRequest();
		void sl_SearchResults_ShowRequest();
		void sl_Tray_Activated (QSystemTrayIcon::ActivationReason reason);
		void sl_Clipboard_DataChanged();
		void sl_EditMenuContentChanged();

		// Menu event handlers
		void sl_NewDocumentAction_Triggered();
		void sl_OpenDocumentAction_Triggered();
		void sl_SaveDocumentAction_Triggered(bool* actionCancelled = 0);
		void sl_SaveDocumentAsAction_Triggered();
		void sl_CloseDocumentAction_Triggered(bool* actionCancelled = 0);
		void sl_DocumentPropertiesAction_Triggered();
		void sl_GlobalSearchAction_Triggered();
		void sl_ExitAction_Triggered();

		void sl_ShowToolbarAction_Triggered();
		void sl_ShowStatusbarAction_Triggered();
		void sl_ApplicationSettingsAction_Triggered();

		void sl_AboutProgramAction_Triggered();
		void sl_AboutQtAction_Triggered();

		void sl_ShowHideMainWindowAction_Triggered();
		void sl_QuickNoteAction_Triggered();

		// Application event handlers
		void sl_CurrentDocument_Changed();
		void sl_Application_CurrentDocumentChanged(Document*);
		void sl_Application_NoteDeleted(Note*);

		void sl_QApplication_AboutToQuit();

	};
}

#endif // MAINWINDOW_H
