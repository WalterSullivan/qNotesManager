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

#include "hierarchymodel.h"

#include "folder.h"
#include "notemodelitem.h"
#include "foldermodelitem.h"
#include "separatormodelitem.h"
#include "application.h"
#include "document.h"
#include "note.h"
#include "global.h"

#include <QDebug>


using namespace qNotesManager;

HierarchyModel::HierarchyModel(Document* doc) : BaseModel(doc) {
	Folder* rootFolder = doc->GetRoot();
	if (rootFolder == nullptr) {
		WARNING("Null reference");
		return;
	}
	RegisterItem(rootFolder);

	Folder* tempFolder = doc->GetTempFolder();
	if (tempFolder == nullptr) {
		WARNING("Null reference");
		return;
	}
	RegisterItem(tempFolder);

	Folder* trashFolder = doc->GetTrashFolder();
	if (trashFolder == nullptr) {
		WARNING("Null reference");
		return;
	}
	RegisterItem(trashFolder);

	SeparatorModelItem* separatorItem = new SeparatorModelItem();
	BaseModelItem* rootItem = _bridge.value(doc->GetRoot());

	rootItem->AddChild(separatorItem);
	rootItem->AddChild(_bridge.value(tempFolder));
	rootItem->AddChild(_bridge.value(trashFolder));

	SetRootItem(rootItem);
}

void HierarchyModel::RegisterItem(Folder* folder) { // Register folder and all items inside of it
	if (folder == nullptr) {
		WARNING("Null pointer recieved");
		return;
	}
	if (_bridge.contains(folder)) {
		WARNING("Item already registered");
		return;
	}

	QObject::connect(folder, SIGNAL(sg_ItemAdded(AbstractFolderItem* const, int)),
					 this, SLOT(sl_Folder_ItemAdded(AbstractFolderItem* const, int)));
	QObject::connect(folder, SIGNAL(sg_ItemAboutToBeRemoved(AbstractFolderItem*const)),
					 this, SLOT(sl_Folder_ItemAboutToBeRemoved(AbstractFolderItem* const)));
	QObject::connect(folder, SIGNAL(sg_ItemAboutToBeMoved(AbstractFolderItem*const, int, Folder*)),
					 this, SLOT(sl_Folder_ItemAboutToBeMoved(AbstractFolderItem* const, int, Folder*)));
	QObject::connect(folder, SIGNAL(sg_ItemsCollectionAboutToClear()),
					 this, SLOT(sl_Folder_ItemsCollectionCleared()));

	FolderModelItem* fi = new FolderModelItem(folder);
	QObject::connect(fi, SIGNAL(sg_DataChanged(BaseModelItem*)), this, SLOT(sl_Item_DataChanged(BaseModelItem*)));
	BaseModelItem* parent = _bridge.value(folder->GetParent());
	if (parent != nullptr) {
		parent->AddChildTo(fi, folder->GetParent()->Items.IndexOf(folder));
	}
	_bridge.insert(folder, fi);
	itemsUuids.insert(folder->GetUuid(), folder);

	for (int i = 0; i < folder->Items.Count(); ++i) {
		AbstractFolderItem* item = folder->Items.ItemAt(i);
		if (item->GetItemType() == AbstractFolderItem::Type_Folder) {
			Folder* f = dynamic_cast<Folder*>(item);
			RegisterItem(f);
		} else {
			Note* n = dynamic_cast<Note*>(item);
			RegisterItem(n);
		}
	}
}

void HierarchyModel::RegisterItem(Note* note) {
	if (note == nullptr) {
		WARNING("Null pointer recieved");
		return;
	}
	if (_bridge.contains(note)) {
		WARNING("Item already registered");
		return;
	}

	Folder* folder = note->GetParent();
	BaseModelItem* parentItem = _bridge.value(folder);
	NoteModelItem* noteItem = new NoteModelItem(note);
	QObject::connect(noteItem, SIGNAL(sg_DataChanged(BaseModelItem*)),
					 this, SLOT(sl_Item_DataChanged(BaseModelItem*)));
	parentItem->AddChildTo(noteItem, note->GetParent()->Items.IndexOf(note));
	_bridge.insert(note, noteItem);
	itemsUuids.insert(note->GetUuid(), note);
}

void HierarchyModel::UnregisterItem(Folder* folder) {
	if (folder == nullptr) {
		WARNING("Null pointer recieved");
		return;
	}
	if (!_bridge.contains(folder)) {
		WARNING("Item is not registered");
		return;
	}

	QObject::disconnect(folder, 0, this, 0);

	for (int i = 0; i < folder->Items.Count(); ++i) {
		AbstractFolderItem* item = folder->Items.ItemAt(i);
		if (item->GetItemType() == AbstractFolderItem::Type_Folder) {
			Folder* f = dynamic_cast<Folder*>(item);
			UnregisterItem(f);
		} else {
			Note* n = dynamic_cast<Note*>(item);
			UnregisterItem(n);
		}
	}

	BaseModelItem* childItem = _bridge.value(folder);

	Folder* parent = folder->GetParent();
	if (parent != nullptr) {
		if (!_bridge.contains(parent)) {
			WARNING("Item parent is not registered");
		} else {
			BaseModelItem* parentItem = _bridge.value(parent);
			parentItem->RemoveChild(childItem);
		}
	}

	_bridge.remove(folder);
	itemsUuids.remove(folder->GetUuid());

	if (GetDisplayRootItem() == childItem) {SetDisplayRootItem(GetRootItem());}
	if (GetRootItem() == childItem) {SetRootItem(nullptr);}

	delete childItem;
}

void HierarchyModel::UnregisterItem(Note* note) {
	if (note == nullptr) {
		WARNING("Null pointer recieved");
		return;
	}
	if (!_bridge.contains(note)) {
		WARNING("Item is not registered");
		return;
	}

	Folder* parent = note->GetParent();
	if (parent == nullptr) {
		WARNING("Null pointer recieved");
		return;
	}
	if (!_bridge.contains(parent)) {
		WARNING("Item is not registered");
		return;
	}

	BaseModelItem* parentItem = _bridge.value(parent);
	BaseModelItem* childItem = _bridge.value(note);


	QObject::disconnect(note, 0, this, 0);
	parentItem->RemoveChild(childItem);
	_bridge.remove(note);
	itemsUuids.remove(note->GetUuid());
	delete childItem;
}

void HierarchyModel::SetPinnedFolder(Folder* f) {
	if (f == nullptr) {
		BaseModelItem* currentDisplayRootItem = GetDisplayRootItem();

		SetDisplayRootItem(GetRootItem());

		BaseModelItem* parentItem = currentDisplayRootItem->parent();
		if (parentItem != nullptr) {
			QModelIndexList list;
			QModelIndex newItemIndex = createIndex(parentItem->IndexOfChild(currentDisplayRootItem), 0, currentDisplayRootItem);
			list << newItemIndex;
			emit sg_ApplySelection(list);
		}
	} else {
		if (!_bridge.contains(f)) {
			WARNING("Folder is not registered");
			return;
		}
		SetDisplayRootItem(_bridge.value(f));
	}
}

Folder* HierarchyModel::GetPinnedFolder() const {
	const BaseModelItem* displayRootItem = GetDisplayRootItem();

	// No folder pinned
	if (displayRootItem == GetRootItem()) {
		return nullptr;
	}

	if (displayRootItem == nullptr) {
		WARNING("Display root is NULL");
		return nullptr;
	}

	if (displayRootItem->DataType() != BaseModelItem::folder) {
		WARNING("Display root is not a folder");
		return nullptr;
	}

	const FolderModelItem* folderModelItem = dynamic_cast<const FolderModelItem*>(displayRootItem);
	return folderModelItem->GetStoredData();
}

void HierarchyModel::sl_Folder_ItemAdded(AbstractFolderItem* const item, int) {
	Folder* parent = static_cast<Folder*>(QObject::sender());
	if (!_bridge.contains(parent)) {
		WARNING("Unknown sender");
		return;
	}

	BaseModelItem* parentItem = _bridge.value(parent);

	bool insertInVisibleBranch = false;
	if (parentItem == GetDisplayRootItem() ||
		parentItem->IsOffspringOf(GetDisplayRootItem())) {
		insertInVisibleBranch = true;
	}

	QModelIndex parentIndex = QModelIndex();
	if ( insertInVisibleBranch && (parentItem != GetDisplayRootItem()) ) {
		parentIndex = createIndex(parentItem->parent()->IndexOfChild(parentItem), 0, parentItem);
	}

	if (insertInVisibleBranch) {
		beginInsertRows(parentIndex, parent->Items.IndexOf(item), parent->Items.IndexOf(item));
	}

		if (item->GetItemType() == AbstractFolderItem::Type_Folder) {
			Folder* f = dynamic_cast<Folder*>(item);
			RegisterItem(f);
		} else {
			Note* n = dynamic_cast<Note*>(item);
			RegisterItem(n);
		}

	if (insertInVisibleBranch) {
		endInsertRows();

		QModelIndexList list;
		BaseModelItem* newItem = parentItem->ChildAt(parent->Items.IndexOf(item));
		QModelIndex newItemIndex = createIndex(parentItem->IndexOfChild(newItem), 0, newItem);
		list << newItemIndex;
		emit sg_ApplySelection(list);
	}
}

void HierarchyModel::sl_Folder_ItemAboutToBeRemoved(AbstractFolderItem* const item) {
	Folder* parent = static_cast<Folder*>(QObject::sender());
	if (!_bridge.contains(parent)) {
		WARNING("Unknown sender");
		return;
	}

	BaseModelItem* parentItem = _bridge.value(parent);

	bool removeFromVisibleBranch = false;
	if (parentItem == GetDisplayRootItem() ||
		parentItem->IsOffspringOf(GetDisplayRootItem())) {
		removeFromVisibleBranch = true;
	}

	QModelIndex parentIndex = QModelIndex();
	if (removeFromVisibleBranch && (parentItem != GetDisplayRootItem())) {
		parentIndex = createIndex(parentItem->parent()->IndexOfChild(parentItem), 0, parentItem);
	}

	if (removeFromVisibleBranch) {
		beginRemoveRows(parentIndex, parent->Items.IndexOf(item), parent->Items.IndexOf(item));
	}

		if (item->GetItemType() == AbstractFolderItem::Type_Folder) {
			Folder* f = dynamic_cast<Folder*>(item);
			UnregisterItem(f);

		} else {
			Note* n = dynamic_cast<Note*>(item);
			UnregisterItem(n);
		}

	if (removeFromVisibleBranch) {
		endRemoveRows();
	}
}

void HierarchyModel::sl_Folder_ItemAboutToBeMoved(AbstractFolderItem* const item,
												  int newPosition, Folder* newParent) {
	Folder* parent = item->GetParent();
	if (!_bridge.contains(parent)) {
		WARNING("Item is not registered");
		return;
	}
	if (!_bridge.contains(item)) {
		WARNING("Item is not registered");
		return;
	}
	if (!_bridge.contains(newParent)) {
		WARNING("Item is not registered");
		return;
	}

	BaseModelItem* oldParentItem = _bridge.value(parent);
	BaseModelItem* childItem = _bridge.value(item);
	BaseModelItem* newParentItem = _bridge.value(newParent);

	bool removeFromVisibleBranch = false;
	if (oldParentItem == GetDisplayRootItem() ||
		oldParentItem->IsOffspringOf(GetDisplayRootItem())) {
		removeFromVisibleBranch = true;
	}

	QModelIndex oldParentIndex = QModelIndex();
	if (removeFromVisibleBranch && (oldParentItem != GetDisplayRootItem())) {
		oldParentIndex = createIndex(oldParentItem->parent()->IndexOfChild(oldParentItem), 0, oldParentItem);
	}

	int oldPosition = parent->Items.IndexOf(item);
	if (removeFromVisibleBranch) {
		beginRemoveRows (oldParentIndex, oldPosition, oldPosition);
	}
		oldParentItem->RemoveChild(childItem);

	if (removeFromVisibleBranch) {
		endRemoveRows();
	}

	bool insertInVisibleBranch = false;
	if (newParentItem == GetDisplayRootItem() ||
		newParentItem->IsOffspringOf(GetDisplayRootItem())) {
		insertInVisibleBranch = true;
	}

	// Create model index for new item parent
	QModelIndex newParentIndex = QModelIndex();
	if (parent == newParent) {
		newParentIndex = oldParentIndex;
	} else {
		if (insertInVisibleBranch && (newParentItem != GetDisplayRootItem())) {
			newParentIndex = createIndex(newParentItem->parent()->IndexOfChild(newParentItem), 0, newParentItem);
		}
	}

	// Insert the item
	if (insertInVisibleBranch) {
		beginInsertRows(newParentIndex, newPosition, newPosition);
	}
		newParentItem->AddChildTo(childItem, newPosition);

	if (insertInVisibleBranch) {
		endInsertRows();
	}
}

void HierarchyModel::sl_Folder_ItemsCollectionCleared() {
	// TODO: Implement this
	WARNING("Not implemented");
}

void HierarchyModel::sl_Item_DataChanged(BaseModelItem* modelItem) {
	bool itemInVisibleBranch = false;
	if (modelItem == GetDisplayRootItem() ||
		modelItem->IsOffspringOf(GetDisplayRootItem())) {
		itemInVisibleBranch = true;
	}

	if (!itemInVisibleBranch) {return;}

	QModelIndex itemIndex;
	if (modelItem->parent() == nullptr) {
		itemIndex = QModelIndex();
	} else {
		itemIndex = createIndex(modelItem->parent()->IndexOfChild(modelItem), 0, modelItem);
	}

	emit dataChanged(itemIndex, itemIndex);
}

Qt::DropActions HierarchyModel::supportedDropActions () const {
	return Qt::MoveAction;
}

Qt::ItemFlags HierarchyModel::flags (const QModelIndex& index ) const {
	Qt::ItemFlags defaultFlags = BaseModel::flags(index);
	Qt::ItemFlags returnFlags = defaultFlags;

	if (index.isValid()) {
		BaseModelItem* modelItem = static_cast<BaseModelItem*>(index.internalPointer());
		if (modelItem->DataType() == BaseModelItem::folder && Application::I()->CurrentDocument() != nullptr) {
			FolderModelItem* folderItem = dynamic_cast<FolderModelItem*>(modelItem);
			if (folderItem->GetStoredData() == Application::I()->CurrentDocument()->GetTempFolder() ||
				folderItem->GetStoredData() == Application::I()->CurrentDocument()->GetTrashFolder()) {
				returnFlags = Qt::ItemIsDropEnabled | defaultFlags;
			} else {
				returnFlags = Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
			}
		} else if (modelItem->DataType() == BaseModelItem::note) {
			returnFlags = Qt::ItemIsDragEnabled | defaultFlags;
		}
	} else {
		returnFlags = Qt::ItemIsDropEnabled | defaultFlags;
	}

	return returnFlags;
}

bool HierarchyModel::dropMimeData (const QMimeData* data, Qt::DropAction action,
								   int row, int column, const QModelIndex& parent) {

	qDebug() << "Drop happened";

	if (action == Qt::IgnoreAction) {return true;}
	if (!data->hasFormat("application/qnm-item-uuid-list")) {return false;}
	if (column > 0) {return false;}

	BaseModelItem* newParentModelItem = nullptr;
	if (parent.isValid()) {
		newParentModelItem = static_cast<BaseModelItem*>(parent.internalPointer());
	} else { // dropped to root item
		newParentModelItem = GetDisplayRootItem(); // !
	}

	qDebug() << "To item '" << newParentModelItem->data(Qt::DisplayRole).toString() << "'";
	qDebug() << "At row " << row << ", column " << column << "\n";
	//return false;

	// Modify row variable to ensure 3 system items are always at the botton of the list
	// Not a final version, just dirty hack
	if (newParentModelItem == GetRootItem() && row >= 0 && row > GetRootItem()->ChildrenCount() - 3) {
		row = GetRootItem()->ChildrenCount() - 3;
	}

	if (newParentModelItem->DataType() != BaseModelItem::folder) {
		qWarning() << "Item has wrong type";
		return false;
	}

	QByteArray encodedData = data->data("application/qnm-item-uuid-list");
	QDataStream stream(&encodedData, QIODevice::ReadOnly);

	QList<BaseModelItem*> movedItems;

	while (!stream.atEnd()) {
		int idLen = 0;
		char* c_str = nullptr;

		stream.readRawData((char*)&idLen, sizeof(idLen));
		c_str = new char[idLen];
		stream.readRawData(c_str, idLen);
		QUuid id(c_str);
		delete []c_str;
		if (id.isNull()) {
			qDebug() << "UUDI parsed from mime data is not valid. Skipping";
			continue;
		}


		QHash<QUuid, AbstractFolderItem*>::iterator f_it = itemsUuids.find(id);
		if (f_it == itemsUuids.end()) {
			qWarning() << "Cannot find folder item by id";
			continue;
		}
		AbstractFolderItem* droppedFolderItem = f_it.value();

		QHash<AbstractFolderItem*, BaseModelItem*>::iterator m_it = _bridge.find(droppedFolderItem);
		if (m_it == _bridge.end()) {
			qDebug() << "Cannot find model item by its folder";
			continue;
		}
		BaseModelItem* droppedModelItem = m_it.value();

		movedItems << droppedModelItem;

		Folder* newParentFolder = (dynamic_cast<FolderModelItem*>(newParentModelItem))->GetStoredData();


		if (droppedFolderItem == nullptr) {
			qWarning() << "Casting error";
			continue;
		}

		// Do not allow to drop item to it's own offspring
		if (droppedFolderItem->GetItemType() == AbstractFolderItem::Type_Folder
			&&
			newParentFolder->IsOffspringOf(dynamic_cast<Folder*>(droppedFolderItem))) {

			continue;
		}

		// Moving
		if (droppedFolderItem->GetParent() == newParentFolder) {
			// Moved inside of the parent
			if (-1 == row) {
				// when dropped directly on parent folder, do nothing
			} else if (newParentFolder->Items.IndexOf(droppedFolderItem) == row
					   ||
					   newParentFolder->Items.IndexOf(droppedFolderItem) + 1 == row) {
				// not moved, do nothing
			} else {
				// item moved inside parent folder to another row
				// If new row index is greater than current item index, decrement it. It should be
				// done because Move operation is insertItem(takeItem(i)), so when an item is taken
				// from a list, it's size gets decremented and 'row' variable becomes invalid.
				int actualRow = row > newParentFolder->Items.IndexOf(droppedFolderItem) ? row - 1 : row;
				newParentFolder->Items.Move(droppedFolderItem, actualRow);
			}
		} else { // Moved outside of the parent
			int newPosition = row == -1 ? newParentFolder->Items.Count() : row;
			droppedFolderItem->GetParent()->Items.Move(droppedFolderItem, newPosition, newParentFolder);
		}
	}

	// Send a signal for views to select moved items
	QModelIndexList indexesToSelect;
	foreach (BaseModelItem* movedItem, movedItems) {
		QModelIndex movedItemIndex;
		if (movedItem->parent() == nullptr) {
			movedItemIndex = QModelIndex();
		} else {
			movedItemIndex = createIndex(movedItem->parent()->IndexOfChild(movedItem), 0, movedItem);
		}
		indexesToSelect << movedItemIndex;
	}
	emit sg_ApplySelection(indexesToSelect);


	return true;
}

QMimeData* HierarchyModel::mimeData (const QModelIndexList& indexes) const {
	QMimeData *mimeData = new QMimeData();
	QByteArray encodedData;

	QDataStream stream(&encodedData, QIODevice::WriteOnly);

	QListIterator<QModelIndex> iterator(indexes);
	while (iterator.hasNext()) {
		QModelIndex index = iterator.next();
		if (index.isValid()) {
			BaseModelItem* baseModelItem = static_cast<BaseModelItem*>(index.internalPointer());
			if (baseModelItem == nullptr) {continue;}

			AbstractFolderItem* folderItem = nullptr;
			if (baseModelItem->DataType() == BaseModelItem::folder) {
				folderItem = (dynamic_cast<FolderModelItem*>(baseModelItem))->GetStoredData();
			} else if (baseModelItem->DataType() == BaseModelItem::note) {
				folderItem = (dynamic_cast<NoteModelItem*>(baseModelItem))->GetStoredData();
			}
			if (folderItem == nullptr) {continue;}

			QUuid id = folderItem->GetUuid();
			QString idStr = id.toString();
			int idLen = idStr.length();

			stream.writeRawData((char*)&idLen, sizeof(idLen));
			stream.writeRawData(idStr.toStdString().c_str(), idLen);
		}
	}

	mimeData->setData("application/qnm-item-uuid-list", encodedData);
	return mimeData;
}

QStringList HierarchyModel::mimeTypes () const {
	QStringList types;
	types << "application/qnm-item-uuid-list";
	return types;
}

void HierarchyModel::sl_RequestEmitApplySelection(AbstractFolderItem* item) {
	if (item == nullptr) {return;}

	BaseModelItem* modelItem = _bridge.value(item);
	BaseModelItem* parentItem = modelItem->parent();

	if (parentItem == nullptr) {return;}

	QModelIndexList list;
	QModelIndex newItemIndex = createIndex(parentItem->IndexOfChild(modelItem), 0, modelItem);
	list << newItemIndex;
	emit sg_ApplySelection(list);
}
