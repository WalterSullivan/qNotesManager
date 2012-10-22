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

#include "folderitemcollection.h"
#include "abstractfolderitem.h"
#include "folder.h"

using namespace qNotesManager;

FolderItemCollection::FolderItemCollection(Folder* owner) : _owner(owner) {
	Q_ASSERT(owner != 0);
}

void FolderItemCollection::Add(AbstractFolderItem* item) {
	Q_ASSERT(item != 0);
	Q_ASSERT(item != _owner);

	AddTo(item, _items.size());
}

void FolderItemCollection::AddTo(AbstractFolderItem* item, int index) {
	Q_ASSERT(item != 0);
	Q_ASSERT(item != _owner);
	Q_ASSERT(!_items.contains(item));
	Q_ASSERT(item->GetParent() == 0);
	Q_ASSERT(index >= 0 && index <= _items.size());

	emit sg_ItemAboutToBeAdded(item, index);

	_items.insert(index, item);
	item->SetParent(_owner);

	emit sg_ItemAdded(item, index);
}

void FolderItemCollection::Remove(AbstractFolderItem* item) {
	Q_ASSERT(item != 0);
	Q_ASSERT(_items.contains(item));
	Q_ASSERT(item->GetParent() == this->_owner);

	emit sg_ItemAboutToBeRemoved(item);

	_items.removeOne(item);
	item->SetParent(0);

	emit sg_ItemRemoved(item);
}

bool FolderItemCollection::Contains(AbstractFolderItem* item) const {
	Q_ASSERT(item != 0);

	return _items.contains(item);
}

void FolderItemCollection::Clear() {
	for (int i = 0; i < _items.count(); ++i) {
		_items.at(i)->SetParent(0);
	}

	emit sg_AboutToClear();
	_items.clear();
	emit sg_Cleared();
}

void FolderItemCollection::Move(int from, int to) {
	Q_ASSERT(from >= 0 && from < _items.size());
	Q_ASSERT(to >= 0 && to < _items.size());

	Move(_items.at(from), to, _owner);
}

void FolderItemCollection::Move(AbstractFolderItem* item, int to) {
	Q_ASSERT(item != 0);
	Q_ASSERT(_items.contains(item));
	Q_ASSERT(to >= 0 && to < _items.size());

	Move(item, to, _owner);
}

void FolderItemCollection::Move(AbstractFolderItem* item, Folder* newParent) {
	Q_ASSERT(item != 0);
	Q_ASSERT(_items.contains(item));
	Q_ASSERT(newParent != 0);

	int to = newParent->Items.Count();
	if (_owner == newParent) {
		to = _items.count() - 1;
	}
	Move(item, to, newParent);
}

void FolderItemCollection::Move(AbstractFolderItem* item, int to, Folder* newParent) {
	Q_ASSERT(item != 0);
	Q_ASSERT(_items.contains(item));
	Q_ASSERT(to >= 0);
	Q_ASSERT(newParent != 0);

	//if (from == to) {return;}

	emit sg_ItemAboutToBeMoved(item, to, newParent);

	if (_owner == newParent ) {
		Q_ASSERT( ( (to == _items.count()) && (_items.count() == 0) ) ||
				 ( (to < _items.count()) && (_items.count() > 0) ) );
		_items.move(_items.indexOf(item), to);
	} else {
		Q_ASSERT(to <= newParent->Items.Count());
		_items.removeAll(item);
		newParent ->Items._items.insert(to, item);
		item->SetParent(newParent );
	}

	emit sg_ItemMoved(item, to, newParent);
}

int FolderItemCollection::Count() const {
	return _items.count();
}

AbstractFolderItem* FolderItemCollection::ItemAt(int index) const {
	Q_ASSERT(index >= 0 && index < _items.count());

	return _items.at(index);
}

int FolderItemCollection::IndexOf(AbstractFolderItem* const item) const {
	Q_ASSERT(item != 0);
	Q_ASSERT(_items.contains(item));

	return _items.indexOf(item);
}
