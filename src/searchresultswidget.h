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

#ifndef SEARCHRESULTSWIDGET_H
#define SEARCHRESULTSWIDGET_H

#include "searchresultsmodel.h"

#include <QWidget>
#include <QTreeView>
#include <QPushButton>
#include <QProgressBar>
#include <QAction>
#include <QMenu>

namespace qNotesManager {
	class DocumentSearchEngine;

	class SearchResultsWidget : public QWidget {
	Q_OBJECT
	private:
		QTreeView* treeView;
		QPushButton* clearButton;
		QPushButton* expandAllButton;
		QPushButton* stopSearchButton;
		QPushButton* closeButton;
		QProgressBar* progressBar;
		SearchResultsModel* searchResultsModel;

		QMenu* contextMenu;
		QAction* showInTreeAction;

	public:
		explicit SearchResultsWidget(DocumentSearchEngine* eng, QWidget *parent = 0);

		DocumentSearchEngine*	engine;

		void ClearResults();

	signals:
		void sg_ShowSearchResults(NoteFragment);
		void sg_CloseRequest();
		void sg_ShowRequest();
		void sg_NoteHighlightRequest(Note*);

	private slots:
		void sl_ClearButton_Clicked();
		void sl_ExpandAllButton_Clicked();
		void sl_StopSearchButton_Clicked();

		void sl_SearchStarted();
		void sl_SearchEnded();
		void sl_SearchProgress(int);
		void sl_SearchResult(const NoteFragment&);

		void sl_Document_NoteDeleted(Note*);
		void sl_ListView_DoubleClicked(const QModelIndex&);
		void sl_TreeView_ContextMenuRequested(const QPoint&);

		void sl_ShowInTreeAction_Triggered();

	};
}

#endif // SEARCHRESULTSWIDGET_H
