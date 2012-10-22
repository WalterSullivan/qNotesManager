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

#ifndef SEPARATORMODELITEM_H
#define SEPARATORMODELITEM_H

#include "basemodelitem.h"

namespace qNotesManager {
	class SeparatorModelItem : public BaseModelItem {
	Q_OBJECT
	public:
		explicit SeparatorModelItem();
		/*virtual*/ Qt::ItemFlags flags () const;
		/*virtual*/ void AddChild(BaseModelItem* n);
		/*virtual*/ void AddChildTo(BaseModelItem* n, int position);
		/*virtual*/ void RemoveChild(BaseModelItem* n);
	};
}

#endif // SEPARATORMODELITEM_H
