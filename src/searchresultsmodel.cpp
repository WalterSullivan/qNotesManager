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

#include "searchresultsmodel.h"

#include "notemodelitem.h"
#include "searchmodelitem.h"
#include "note.h"
#include "global.h"

#include <QDebug>

using namespace qNotesManager;

SearchResultsModel::SearchResultsModel(QObject *parent) : BaseModel(parent) {
	BaseModelItem* root = new BaseModelItem();
	root->SetSorted(true);
	SetRootItem(root);
}

void SearchResultsModel::AddResult(const NoteFragment& fragment) {
	NoteModelItem* noteItem = 0;

	if (notesHash.contains(fragment.NotePtr)) {
		noteItem = notesHash.value(fragment.NotePtr);
	} else {
		Note* note = const_cast<Note*>(fragment.NotePtr);
		if (!note) {
			WARNING("Null pointer recieved");
			return;
		}
		noteItem = new NoteModelItem(note);
		noteItem->IsEditable = false;
		notesHash.insert(fragment.NotePtr, noteItem);

		QObject::connect(noteItem, SIGNAL(sg_DataChanged(BaseModelItem*)),
						 this, SLOT(sl_Item_DataChanged(BaseModelItem*)));

		int newPosition = GetRootItem()->FindInsertIndex(noteItem);

		beginInsertRows(QModelIndex(), newPosition, newPosition);
			GetRootItem()->AddChildTo(noteItem, newPosition);
		endInsertRows();
	}

	// Add search result to noteItem
	SearchModelItem* searchResultItem = new SearchModelItem(fragment);
	QObject::connect(searchResultItem, SIGNAL(sg_DataChanged(BaseModelItem*)),
					 this, SLOT(sl_Item_DataChanged(BaseModelItem*)));
	resultsHash.insert(fragment.NotePtr, searchResultItem);

	QModelIndex noteItemIndex = createIndex(GetRootItem()->IndexOfChild(noteItem), 0, noteItem);
	beginInsertRows(noteItemIndex, noteItem->ChildrenCount(), noteItem->ChildrenCount());
		noteItem->AddChild(searchResultItem);
	endInsertRows();
}

void SearchResultsModel::RemoveResultsForNote(const Note* n) {
	if (!n) {
		WARNING("Null pointer recieved");
		return;
	}
	if (!notesHash.contains(n)) {return;}

	NoteModelItem* noteItem = notesHash.value(n);
	notesHash.remove(n);
	resultsHash.remove(n);

	beginRemoveRows(QModelIndex(), GetRootItem()->IndexOfChild(noteItem), GetRootItem()->IndexOfChild(noteItem));
		GetRootItem()->RemoveChild(noteItem);
	endRemoveRows();

	delete noteItem;
}

void SearchResultsModel::ClearResults() {
	if (GetRootItem()->ChildrenCount() == 0) {return;}
	beginRemoveRows(QModelIndex(), 0, GetRootItem()->ChildrenCount() - 1);
		foreach (const Note* n, notesHash.keys()) {
			NoteModelItem* item = notesHash.value(n);
			GetRootItem()->RemoveChild(item);
			delete item;
		}
	endRemoveRows();

	notesHash.clear();
	resultsHash.clear();
}

bool SearchResultsModel::ContainsResultForNote(const Note* note) {
	if (!note) {
		WARNING("Null pointer recieved");
		return false;
	}
	return notesHash.contains(note);
}

void SearchResultsModel::sl_Item_DataChanged(BaseModelItem* item) {
	QModelIndex noteItemIndex = createIndex(item->parent()->IndexOfChild(item), 0, item);
	emit dataChanged(noteItemIndex, noteItemIndex);
}
