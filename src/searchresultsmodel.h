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

#ifndef SEARCHRESULTSMODEL_H
#define SEARCHRESULTSMODEL_H

#include "basemodel.h"
#include "notefragment.h"

#include <QMultiHash>

namespace qNotesManager {
	class NoteModelItem;
	class SearchModelItem;

	class SearchResultsModel : public BaseModel {
	Q_OBJECT
	private:
		QHash<const Note*, NoteModelItem*> notesHash;
		QMultiHash<const Note*, SearchModelItem*> resultsHash;

	public:
		explicit SearchResultsModel(QObject *parent = 0);

		void AddResult(const NoteFragment&);
		void RemoveResultsForNote(const Note*);
		bool ContainsResultForNote(const Note*);
		void ClearResults();
		void UpdateEntry(const Note*);

	private slots:
		void sl_Item_DataChanged(BaseModelItem*);
	};
}

#endif // SEARCHRESULTSMODEL_H
