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

#include "basemodel.h"
#include "basemodelitem.h"
#include "global.h"

using namespace qNotesManager;

BaseModel::BaseModel(QObject *parent) : QAbstractItemModel(parent) {
	displayRootItem = nullptr;
	rootItem = nullptr;
}

/*virtual*/
BaseModel::~BaseModel() {
}

void BaseModel::SetDisplayRootItem(BaseModelItem* item) {
	if (item == displayRootItem) {return;}

	// # Check if item belongs to current model's hierarchy
	if (item != nullptr) {
		BaseModelItem* parent = item;
		while(parent->parent() != nullptr) {
			parent = parent->parent();
		}
		if (parent != rootItem) {
			WARNING("Item doesn't belong to model hierarchy");
			return;
		}
	}

	if (displayRootItem != nullptr && (displayRootItem->ChildrenCount() > 0)) {
		beginRemoveRows(QModelIndex(), 0, displayRootItem->ChildrenCount() - 1);
			displayRootItem = nullptr;
		endRemoveRows();
	}

	if (item != nullptr && (item->ChildrenCount() > 0)) {
		beginInsertRows(QModelIndex(), 0, item->ChildrenCount() - 1);
			displayRootItem = item;
		endInsertRows();
	} else {
		displayRootItem = item;
	}

	emit sg_DisplayRootItemChanged();
}

BaseModelItem* BaseModel::GetDisplayRootItem() const {
	return displayRootItem;
}

void BaseModel::SetRootItem(BaseModelItem* item) {
	if (rootItem == item) {return;}

	if (rootItem) {
		rootItem->setParent(nullptr);
	}

	rootItem = item;
	SetDisplayRootItem(rootItem);

	item->setParent(this); // QObject parentship
}

BaseModelItem* BaseModel::GetRootItem() const {
	return rootItem;
}

/*virtual*/
QModelIndex BaseModel::index(int row, int column, const QModelIndex& parent) const {
	if (!hasIndex(row, column, parent)) {
		return QModelIndex();
	}

	if (!parent.isValid()) {
		return createIndex(row, column, displayRootItem->ChildAt(row) );
	}

	BaseModelItem* item = static_cast<BaseModelItem*>(parent.internalPointer());
	return createIndex(row, column, item->ChildAt(row));
}

/*virtual*/
QModelIndex BaseModel::parent(const QModelIndex& child) const {
	if (!child.isValid()) {return QModelIndex();}

	BaseModelItem* item = static_cast<BaseModelItem*>(child.internalPointer());

	if (item->parent() == nullptr || item->parent() == displayRootItem) {return QModelIndex();}

	BaseModelItem* parentItem = item->parent();

	int row = 0;
	if (parentItem->parent() != nullptr) {
		row = parentItem->parent()->IndexOfChild(parentItem);
	}
	return createIndex(row, 0, parentItem);
}

/*virtual*/
int BaseModel::rowCount(const QModelIndex& parent) const {
	if (displayRootItem == nullptr) {return 0;}

	if (!parent.isValid()) {
		return displayRootItem->ChildrenCount();
	}

	BaseModelItem* item = static_cast<BaseModelItem*>(parent.internalPointer());
	return item->ChildrenCount();
}

/*virtual*/
int BaseModel::columnCount(const QModelIndex& parent) const {
	(void)parent;
	return 1;
}

/*virtual*/
QVariant BaseModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid()) {return QVariant();}

	BaseModelItem* item = static_cast<BaseModelItem*>(index.internalPointer());
	return item->data(role);
}

/*virtual*/
bool BaseModel::setData(const QModelIndex& index, const QVariant& value,
						int role) {
	if (!index.isValid()) {return false;}

	BaseModelItem* item = static_cast<BaseModelItem*>(index.internalPointer());
	return item->setData(value, role);
}

/* virtual */
Qt::ItemFlags BaseModel::flags (const QModelIndex& index) const {
	if (!index.isValid()) {
		return Qt::NoItemFlags;
	} else {
		BaseModelItem* modelItem = static_cast<BaseModelItem*>(index.internalPointer());
		return modelItem->flags();
	}
}

void BaseModel::EmitDataChanged(QModelIndex& startIndex, QModelIndex& endIndex) {
	emit dataChanged(startIndex, endIndex);
}

