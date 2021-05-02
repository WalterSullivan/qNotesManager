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

#ifndef ABSTRACTFOLDERITEM_H
#define ABSTRACTFOLDERITEM_H

#include <QObject>
#include <QUuid>

/*
AbstractFolderItem is an interface for items, that Folder may contain
*/

namespace qNotesManager {
	class Folder;

	class AbstractFolderItem : public QObject {
	Q_OBJECT

	friend class FolderItemCollection;

	public:
		virtual ~AbstractFolderItem();

		enum ItemType {
			Type_Folder,
			Type_Note
		};

		ItemType GetItemType() const;
		Folder* GetParent() const;
		bool IsOffspringOf(const Folder*) const;
		QUuid GetUuid() const;

	private:
		Folder* parent;
		const ItemType itemType;
		const QUuid uuid;

		void SetParent(Folder*);

	protected:
		explicit AbstractFolderItem(ItemType type); // Make it unable to create an instance of this class

	signals:
		void sg_ParentChanged(const Folder* newParent);
	};
}

#endif // ABSTRACTFOLDERITEM_H
