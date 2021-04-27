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

#ifndef NAVIGATIONPANELWIDGET_H
#define NAVIGATIONPANELWIDGET_H

#include <QWidget>
#include <QTabWidget>

namespace qNotesManager {
	class FolderNavigationWidget;
	class TagsNavigationWidget;
	class DateNavigationWidget;
	class Note;
	class Document;

	class NavigationPanelWidget : public QWidget {
	Q_OBJECT
	private:
		QTabWidget*					tabWidget;

		FolderNavigationWidget*		hierarchyWidget;
		TagsNavigationWidget*		tagsWidget;
		DateNavigationWidget*		datesWidget;

	public:
		explicit NavigationPanelWidget(QWidget *parent = nullptr);
		int CurrentTabIndex() const;
		void SetCurrentTab(int index);
		void SetTargetDocument(Document*);
		QList<QAction*> GetSelectedItemsActions() const;
		void UpdateViewsVisibility();


	signals:
		void sg_NoteClicked(Note*);
		void sg_NoteDoubleClicked(Note*);
		void sg_SelectedItemsActionsListChanged();

	private slots:
		void sl_TabWidget_CurrentChanged(int);

	public slots:
		void sl_SelectNoteInTree(Note*, bool activateNavigationTab = false);

	};
}

#endif // NAVIGATIONPANELWIDGET_H
