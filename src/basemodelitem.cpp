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

#include "basemodelitem.h"
#include "basemodel.h"

#ifdef DEBUG
	#include <QDebug>
#endif

using namespace qNotesManager;

BaseModelItem::BaseModelItem(ItemType type)
	: itemType(type)
{
	parentItem = 0;
	sorted = false;
	sortOrder = Qt::AscendingOrder;
}

BaseModelItem::~BaseModelItem() {
	Clear();
}

// Return item parent or 0 if there is none.
BaseModelItem* BaseModelItem::parent() const {
	return parentItem;
}

/* virtual */
// Returns item flags
Qt::ItemFlags BaseModelItem::flags () const {
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

// Adds item 'item' into the end children list.
// It is supposed that 'item' is a valid pointer.
void BaseModelItem::AddChild(BaseModelItem* item) {
	Q_ASSERT(item != 0);
	Q_ASSERT(!childrenList.contains(item));
	Q_ASSERT(item->parentItem == 0);


	AddChildTo(item, childrenList.size());
}

// Adds item 'item' into children list in specified position.
// It is supposed that 'item' is a valid pointer.
// NOTE: 'position' parameter is ignored when sorting is enabled
void BaseModelItem::AddChildTo(BaseModelItem* item, int position) {
	Q_ASSERT(item != 0);
	Q_ASSERT(!childrenList.contains(item));
	Q_ASSERT(item->parentItem == 0);
	Q_ASSERT(position >= 0 && position <= childrenList.size());

	childrenList.insert(position, item);
	item->parentItem = this;
	insertIndexCache.Clear();
}

// Removes child item
// It is supposed that 'item' is a valid pointer.
void BaseModelItem::RemoveChild(BaseModelItem* item) {
	Q_ASSERT(item != 0);
	Q_ASSERT(childrenList.contains(item));

	childrenList.removeAll(item);
	item->parentItem = 0;
	insertIndexCache.Clear();
}

// Returns index of child item. It is supposed that 'item' is actually child item.
// It is supposed that 'item' is a valid pointer.
int BaseModelItem::IndexOfChild(BaseModelItem* item) const {
	Q_ASSERT(item != 0);
	Q_ASSERT(childrenList.contains(item));

	return childrenList.indexOf(item);
}

// Returns children count
int BaseModelItem::ChildrenCount() const {
	return childrenList.count();
}

// Returns child item at position 'index'. It is supposed that 'index' is valid index
BaseModelItem* BaseModelItem::ChildAt(int index) const {
	if (!(index >= 0 && index < childrenList.count())) {
		qWarning("Item at non-existent index requested");
		return 0;
	}


	return childrenList.at(index);
}

void BaseModelItem::Clear() {
	for (int i = 0; i < childrenList.count(); ++i) {
		delete childrenList.at(i);
		childrenList[i] = 0;
	}
	childrenList.clear();
	insertIndexCache.Clear();
}

// Returns true if object is a child or a grandchild of item 'parent', otherwise returns false.
// It is supposed that 'parent' is a valid pointer.
bool BaseModelItem::IsOffspringOf(const BaseModelItem* parent) const {
	Q_ASSERT(parent != 0);
	if (parent == this) {return false;}

	const BaseModelItem* f = parent;
	while (f != 0) {
		if (f == parent) {return true;}
		f = f->parent();
	}
	return false;
}

BaseModelItem::ItemType BaseModelItem::DataType() const {
	return itemType;
}

QVariant BaseModelItem::data(int role) const {
	return QVariant();
}

// Returns index where a new item will be inserted with AddChild()
// It is supposed that 'item' is a valid pointer.
int BaseModelItem::FindInsertIndex(const BaseModelItem* item) const {
	Q_ASSERT(item != 0);
	int newIndex = -1;
	// Cached index. Mechanism not finished
	//if (insertIndexCache.IsValid() && insertIndexCache.prt == item) {
		//return insertIndexCache.cachedIndex;
	//}

	if (sorted) {
		newIndex = findInsertIndex_Sorted(item);
	} else {
		newIndex = findInsertIndex_Simple(item);
	}

	//insertIndexCache.Set(item, newIndex);

	return newIndex;
}

//int BaseModelItem::FindInsertIndex(const BaseModelItem* item, const int suggestedIndex) const {}

// virtual
// Compares internal item data for sorting purpose. Returns true by default.
// It is supposed that 'item' is a valid pointer.
bool BaseModelItem::LessThan(const BaseModelItem*) const {
	return true;
}

// Returns index for a new item when list is sorted
// It is supposed that 'item' is a valid pointer.
int BaseModelItem::findInsertIndex_Sorted(const BaseModelItem* item) const {
	// TODO: add anchors handling
	// TODO: handle Qt::DescendingOrder
	Q_ASSERT(item != 0);

	const int size = ChildrenCount();

	if (size == 0) {return 0;}
	if (item->LessThan(ChildAt(0))) {return 0;}
	if (ChildAt(size - 1)->LessThan(item)) {return size;}

	int index = 0;
	int left = 1;
	int right = size - 1;

	while (true) {
		index = (left + right) / 2;

		if (index == 0 || index == size) {
			break;
		}

		BaseModelItem* leftItem = ChildAt(index - 1);
		BaseModelItem* rightItem = ChildAt(index);

		if (leftItem->LessThan(item) && item->LessThan(rightItem)) {
			break;
		} else if (rightItem->LessThan(item)) {
			left = index + 1;
		} else if (item->LessThan(leftItem)) {
			right = index - 1;
		} else {
			break;
		}
	}

	return index;
}

// Returns index for a new item when list is not sorted
// It is supposed that 'item' is a valid pointer.
int BaseModelItem::findInsertIndex_Simple(const BaseModelItem* item) const {
	// TODO: add anchors handling
	return ChildrenCount();
}

// Returns true if children list is sorted, otherwise returns false
bool BaseModelItem::IsSorted() const {
	return sorted;
}

// Sets children list sorting
// NOTE: Sorting may be changed only when children list is empty
void BaseModelItem::SetSorted(const bool s) {
	if (ChildrenCount() == 0) {
		sorted = s;
	}
}

// Returns children list sorting order
Qt::SortOrder BaseModelItem::GetSortOrder() const {
	return sortOrder;
}

// Sets children list sorting order
// NOTE: Sort order may be changed only when children list is empty
void BaseModelItem::SetSortOrder(const Qt::SortOrder order) {
	if (ChildrenCount() == 0) {
		sortOrder = order;
	}
}
