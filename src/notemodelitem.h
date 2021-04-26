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

#ifndef NOTEMODELITEM_H
#define NOTEMODELITEM_H

#include "basemodelitem.h"

namespace qNotesManager {
	class Note;

	class NoteModelItem : public BaseModelItem {
	Q_OBJECT
	private:
		Note*		_storedData;
		void drawLock(QPixmap&) const;

	public:
		explicit NoteModelItem(Note*);

		Note*	GetStoredData() const;

		/*virtual*/
		QVariant data(int role) const;
		/*virtual*/
		bool setData(const QVariant& value, int role);
		/*virtual*/
		Qt::ItemFlags flags () const;
		/*virtual*/
		bool LessThan(const BaseModelItem*) const;

		bool IsEditable;

	private slots:
		void sl_Note_PropertiesChanged();
	};
}

#endif // NOTEMODELITEM_H
