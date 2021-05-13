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

#ifndef DOCUMENTSEARCHENGINE_H
#define DOCUMENTSEARCHENGINE_H

#include <QObject>

#include "notefragment.h"

namespace qNotesManager {
	class Note;
	class DocumentSearchThread;
	class Document;

	class DocumentSearchEngine : public QObject {
	Q_OBJECT

	private:
		Document* document;
		DocumentSearchThread* const thread;

	public:
		explicit DocumentSearchEngine(QObject* parent);
		~DocumentSearchEngine();

		void StartSearch(QString query, bool matchCase = false, bool searchWholeWord = false,
						 bool useRegexp = false);
		bool IsSearchActive() const;
		void StopSearch();
		bool IsQueryValid(QString query, bool useRegExp) const;
		void SetTargetDocument(Document* doc);

	signals:
		void sg_SearchStarted();
		void sg_SearchEnded();
		void sg_SearchProgress(int);
		void sg_SearchResult(const NoteFragment);
		void sg_SearchError(QString);

	private slots:
		void sl_Document_NoteDeleted(Note*);
		void sl_Document_Destroyed();

	};
}

#endif // DOCUMENTSEARCHENGINE_H
