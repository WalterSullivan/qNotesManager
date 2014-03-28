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

#ifndef HIERARCHYMODEL_H
#define HIERARCHYMODEL_H

#include <QHash>
#include <QMimeData>

#include "basemodel.h"

namespace qNotesManager {
	class Folder;
	class Note;
	class AbstractFolderItem;
	class Document;

	class HierarchyModel : public BaseModel {
	Q_OBJECT
	private:
		QHash<AbstractFolderItem*, BaseModelItem*>	_bridge;

		void RegisterItem(Folder* folder);
		void RegisterItem(Note* note);

		void UnregisterItem(Folder* folder);
		void UnregisterItem(Note* note);

	public:
		explicit HierarchyModel(Document*);

		virtual Qt::DropActions supportedDropActions () const;
		virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;

		virtual bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column,
									const QModelIndex & parent );
		virtual QMimeData* mimeData ( const QModelIndexList & indexes ) const;
		virtual QStringList mimeTypes () const;

		void SetPinnedFolder(Folder*);
		Folder* GetPinnedFolder() const;

	private slots:
		void sl_Folder_ItemAdded(AbstractFolderItem* const, int);
		void sl_Folder_ItemAboutToBeRemoved(AbstractFolderItem* const);
		void sl_Folder_ItemAboutToBeMoved(AbstractFolderItem* const, int, Folder*);
		void sl_Folder_ItemsCollectionCleared();

		void sl_Item_DataChanged(BaseModelItem*);

	public slots:
		void sl_RequestEmitApplySelection(AbstractFolderItem*);

	};
}

#endif // HIERARCHYMODEL_H
