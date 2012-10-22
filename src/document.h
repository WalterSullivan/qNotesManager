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

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QList>
#include <QUuid>
#include <QMap>
#include <QSet>
#include <QStandardItemModel>
#include <QDateTime>

#include "documentvisualsettings.h"
#include "exception.h"

/*
  Document class represents a document that contains all notes, folder, tags and can be saved to
  a certain file
*/

namespace qNotesManager {
	class Tag;
	class Folder;
	class Note;
	class AbstractFolderItem;
	class DocumentSearchEngine;
	class HierarchyModel;
	class TagsModel;
	class DatesModel;

	class Document : public QObject {
	Q_OBJECT
	private:
		bool isChanged;	// Is document changed since last save
		void onChange();	// Event, called when document data changed

		Folder*		rootFolder;	// Root folder. System folder, cannot be deleted.
		Folder*		tempFolder;	// Folder for temporary notes. System, cannot be deleted.
		Folder*		trashFolder;	// Trash folder for deleted items.

		QList<Tag*>		allTags;
		QList<Note*>	allNotes;
		QList<Folder*>	allFolders;

		QMap<QString, Tag*> tagsByName; // to quickly fing tag by name

		void RegisterItem(AbstractFolderItem* const item);
		void UnregisterItem(AbstractFolderItem* const item);
		void RegisterTag(Tag* tag);
		void UnregisterTag(Tag* tag);

		QStandardItemModel* tagsListModel; // used for completers in TagsLineEdit
		HierarchyModel* hierarchyModel;
		TagsModel* tagsModel;
		DatesModel* creationDateModel;
		DatesModel* modificationDateModel;
		DatesModel* textDateModel;

		QHash<QString, QPixmap> customIcons;

		QString fileName; // Document filename
		quint16 fileVersion;
		quint8 compressionLevel;

		quint8 cipherID;
		QByteArray password;

		static const quint16 currentFileVersion = 0x0001;

		QDateTime creationDate;
		QDateTime modificationDate;


	public:
		explicit Document();
		~Document();

		DocumentVisualSettings VisualSettings;

		bool IsChanged() const;	// Returns if document was changed

		static Document* Open(QString fileName);// Static method opens file and returns new document from file content
		void Save(QString name = QString(), quint16 version = 0); // Saves document data

		QString GetFilename() const;

		Folder* GetTempFolder() const;
		Folder* GetTrashFolder() const;
		Folder* GetRoot() const;

		Tag* FindTagByName(QString name) const;

		QAbstractItemModel* GetTagsListModel() const;
		HierarchyModel* GetHierarchyModel() const;
		TagsModel* GetTagsModel() const;
		DatesModel* GetCreationDatesModel() const;
		DatesModel* GetModificationDatesModel() const;
		DatesModel* GetTextDatesModel() const;


		QList<Tag*> GetTagsList() const;
		QList<Note*> GetNotesList() const;

		bool LockFolderItems;

		QStandardItemModel*		customIconsModel;
		void AddCustomIcon(QPixmap, QString);
		void RemoveCustomIcon(QString);
		QPixmap GetItemIcon(const QString) const;

		quint8 GetCompressionLevel() const;
		void SetCompressionLevel(const quint8 level);

		quint8 GetCipherID() const;
		QString GetPassword() const;
		void SetCipherData(const quint8 id, const QString& _password = QString());

		QDateTime GetCreationDate() const;
		QDateTime GetModificationDate() const;

		QString DefaultNoteIcon;
		QString DefaultFolderIcon;

	signals:
		void sg_Changed();	// Emitted when document was changed

		void sg_ItemRegistered(Folder*);
		void sg_ItemRegistered(Note*);
		void sg_ItemRegistered(Tag*);

		void sg_ItemUnregistered(Folder*);
		void sg_ItemUnregistered(Note*);
		void sg_ItemUnregistered(Tag*);

		void sg_SavingProgress(int);
		void sg_LoadingProgress(int);

	private slots:
		void sl_Folder_ItemAdded(AbstractFolderItem* const, int);
		void sl_Folder_ItemRemoved(AbstractFolderItem* const);
		void sl_ItemDataChanged(); // when folder or note or tag were changed

		void sl_Note_TagAdded(Tag*);
		void sl_Note_TagRemoved(Tag*);

	};

	class WrongFileException : public Exception {
	public:
		WrongFileException(QString message, QString description, QString position) :
				Exception(message, description, position) {}
	};

	class WrongFileVersionException : public Exception {
	public:
		WrongFileVersionException(QString message, QString description, QString position) :
				Exception(message, description, position) {}
	};
}

#endif // DOCUMENT_H
