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

#ifndef SEARCHMODELITEM_H
#define SEARCHMODELITEM_H

#include "basemodelitem.h"
#include "notefragment.h"

namespace qNotesManager {
	class SearchModelItem : public BaseModelItem {
	Q_OBJECT
	private:
		const NoteFragment fragment;
		bool expired;

	public:
		SearchModelItem(const NoteFragment& f);
		/*virtual*/ QVariant data(int role) const;
		/*virtual*/ Qt::ItemFlags flags () const;
		NoteFragment Fragment() const;

		static const int HighlightStartRole = 50;
		static const int HightlightLengthRole = 51;

	private slots:
		void sl_Note_TextChanged();
	};
}

#endif // SEARCHMODELITEM_H
