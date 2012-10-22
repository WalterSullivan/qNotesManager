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

#ifndef DOCUMENTSEARCHTHREAD_H
#define DOCUMENTSEARCHTHREAD_H

#include <QThread>
#include <QReadWriteLock>
#include <QRegExp>
#include <QList>

#include "notefragment.h"

namespace qNotesManager {
	class Note;

	class DocumentSearchThread : public QThread {
		Q_OBJECT
		private:
			QRegExp regexp;
			volatile bool isActive;
			const Note* currentNote;
			QList<Note*> searchQueue;

			mutable QReadWriteLock isActiveLock;
			mutable QReadWriteLock listLock;

			int primarySearchQueueSize;
			int processedNotesCount;

			void SetCurrentNote(const Note*);
			void SetActive(bool a);
		protected:
			/*virtual*/ void run();

		public:
			DocumentSearchThread(QObject* parent);

			bool IsActive() const;
			void Deactivate();


			const Note* CurrentNote() const;

			void SetRegexp(const QRegExp& regexp);
			void AddNote(Note*);
			void RemoveNote(Note*);
			void ClearNotesList();

		signals:
			void sg_SearchResult(NoteFragment);
			void sg_SearchStarted();
			void sg_SearchEnded();
			void sg_SearchProgress(int);

	};
}

#endif // DOCUMENTSEARCHTHREAD_H
