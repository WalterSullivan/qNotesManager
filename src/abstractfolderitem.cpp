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

#include "abstractfolderitem.h"
#include "folder.h"
#include "global.h"

using namespace qNotesManager;

AbstractFolderItem::AbstractFolderItem(ItemType type) : QObject(0), itemType(type) {
	parent = 0;
}

AbstractFolderItem::~AbstractFolderItem() {
}

void AbstractFolderItem::SetParent(Folder* newParent) {
	if (newParent == this) {
		WARNING("Trying to set item as it's parent");
		return;
	}

	if (newParent == parent) {return;}

	parent = newParent;
	emit sg_ParentChanged(newParent);
}

AbstractFolderItem::ItemType AbstractFolderItem::GetItemType() const {
	return itemType;
}

Folder* AbstractFolderItem::GetParent() const {
	return parent;
}

bool AbstractFolderItem::IsOffspringOf(const Folder* folder) const {
	if (folder == 0) {
		WARNING("Null reference");
		return false;
	}

	if (folder == this) {return false;}

	Folder* f = parent;
	while (f != 0) {
		if (f == folder) {return true;}
		f = f->GetParent();
	}
	return false;
}
