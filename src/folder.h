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

#ifndef FOLDER_H
#define FOLDER_H

#include "abstractfolderitem.h"
#include "folderitemcollection.h"

#include <QDateTime>
#include <QIcon>

/*
  Folder is item, that may contain notes and another folders
*/

namespace qNotesManager {
	class Folder : public AbstractFolderItem {
	friend class Serializer;
	Q_OBJECT
	public:
		enum FolderType {
			UserFolder = 0,
			SystemFolder = 1,
			TempFolder = 3,
			TrashFolder = 5
		};

	private:
		FolderType		type;
		QString			name;
		const QColor	defaultForeColor;
		const QColor	defaultBackColor;
		QColor			nameForeColor;
		QColor			nameBackColor;
		bool			locked;
		QString			iconID;
		QDateTime		creationDate;
		QDateTime		modificationDate;
		bool			expanded;

	public:
		explicit Folder(QString _name = QString(), FolderType _type = UserFolder);
		~Folder();

		QString GetName() const;
		void SetName (const QString s);

		FolderType GetType() const;
		void SetType(FolderType _type);

		QPixmap GetIcon() const;
		void SetIconID(const QString id);
		QString GetIconID() const;

		QDateTime GetCreationDate() const;
		QDateTime GetModificationDate() const;

		QColor GetNameForeColor() const;
		void SetNameForeColor(const QColor);

		QColor GetNameBackColor() const;
		void SetNameBackColor(const QColor);

		QColor GetDefaultForeColor() const;
		QColor GetDefaultBackColor() const;

		bool IsLocked() const;
		void SetLocked(bool);

		bool IsExpanded() const;
		void SetExpanded(bool);

		QString GetPath() const;

		FolderItemCollection Items;

	signals:
		void sg_VisualPropertiesChanged();
		void sg_DataChanged();

		void sg_ItemAboutToBeAdded(AbstractFolderItem* const, int);
		void sg_ItemAdded(AbstractFolderItem* const, int);
		void sg_ItemAboutToBeRemoved(AbstractFolderItem* const);
		void sg_ItemRemoved(AbstractFolderItem* const);
		void sg_ItemAboutToBeMoved(AbstractFolderItem* const, int, Folder*);
		void sg_ItemMoved(AbstractFolderItem* const, int, Folder*);
		void sg_ItemsCollectionAboutToClear();
		void sg_ItemsCollectionCleared();
	};
}

#endif // FOLDER_H
