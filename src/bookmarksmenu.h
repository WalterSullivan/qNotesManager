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

#ifndef BOOKMARKSMENU_H
#define BOOKMARKSMENU_H

#include <QMenu>

namespace qNotesManager {
	class Document;
	class Note;

	class BookmarksMenu : public QMenu {
	Q_OBJECT
	private:
		Document* currentDocument;
		Note* currentNote;

		QAction* addBookmarkAction;
		QAction* removeBookmarkAction;
		QHash<QAction*, Note*> links;

		void updateMenu();

	public:
		explicit BookmarksMenu(const QString& title, QWidget *parent = nullptr);
		void SetDocument(Document*);

	signals:
		void sg_OpenBookmark(Note*);

	public slots:
		void sl_CurrentNoteChanged(Note* note);

	private slots:
		void sl_AddBookmarkAction_Triggered();
		void sl_RemoveBookmarkAction_Triggered();
		void sl_ActionTriggered(QAction*);
		void sl_Document_BookmarksListChanged();
	};
}
#endif // BOOKMARKSMENU_H
