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
#include <QSemaphore>
#include <QPointer>

#include "documentvisualsettings.h"

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
	class CachedImageFile;

	class Document : public QObject {
	Q_OBJECT
	friend class Serializer;

	private:
		bool inInitMode;
		bool hasUnsavedData;
		bool isModified;
		void onChange();

		Folder*		rootFolder;
		Folder*		tempFolder;
		Folder*		trashFolder;
		Folder*		pinnedFolder;

		QList<Tag*>		allTags;
		QList<Note*>	allNotes;
		QList<Folder*>	allFolders;
		QList<Note*> bookmarks;

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

		QHash<QString, CachedImageFile*> customIcons;

		QString fileName; // Document filename
		quint16 fileVersion;
		quint8 compressionLevel;

		quint8 cipherID;
		QByteArray password;

		QDateTime creationDate;
		QDateTime modificationDate;

		QString DefaultNoteIcon;
		QString DefaultFolderIcon;

		QIcon tagIcon;

		mutable QDateTime fileTimeStamp;
		mutable bool doNotReloadFlag;

        QPointer<QThread> saveThread;

	public:
		explicit Document();
		~Document();

		DocumentVisualSettings VisualSettings;

		bool HasUnsavedData() const;	// Returns if document was changed

		void Open(QString fileName);
		void Save(QString name = QString(), quint16 version = 0);

		QString GetFilename() const;

		Folder* GetTempFolder() const;
		Folder* GetTrashFolder() const;
		Folder* GetRoot() const;

		Folder* GetPinnedFolder() const;
		void SetPinnedFolder(Folder*);

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

		void AddCustomIcon(CachedImageFile*);
		void AddCustomIconToStorage(CachedImageFile*);
		void RemoveCustomIcon(QString);
		QPixmap GetItemIcon(const QString) const;

		quint8 GetCompressionLevel() const;
		void SetCompressionLevel(const quint8 level);

		quint8 GetCipherID() const;
		QString GetPassword() const;
		void SetCipherData(const quint8 id, const QString& _password = QString());

		QDateTime GetCreationDate() const;
		QDateTime GetModificationDate() const;

		QString GetDefaultNoteIcon() const;
		QString GetDefaultFolderIcon() const;
		void SetDefaultNoteIcon(QString id);
		void SetDefaultFolderIcon(QString id);

		int GetBookmarksCount() const;
		Note* GetBookmark(int index) const;
		void AddBookmark(Note*);
		void RemoveBookmark(Note*);
		bool IsBookmark(Note*);

		QDateTime GetFileTimeStamp() const;
		bool DoNotReload() const;
		void SetDoNotReload(bool v) const;

	signals:
		void sg_Changed();	// Emitted when document was changed

		void sg_ItemRegistered(Folder*);
		void sg_ItemRegistered(Note*);
		void sg_ItemRegistered(Tag*);

		void sg_ItemUnregistered(Folder*);
		void sg_ItemUnregistered(Note*);
		void sg_ItemUnregistered(Tag*);

		void sg_BookmarksListChanged();


		// Worker signals
		void sg_LoadingStarted();
		void sg_LoadingProgress(int);
		void sg_LoadingPartiallyFinished();
		void sg_LoadingFinished();
		void sg_LoadingFailed(QString errorString);
		void sg_LoadingAborted();

		void sg_SavingStarted();
		void sg_SavingProgress(int);
		void sg_SavingFinished();
		void sg_SavingFailed(QString errorString);
		void sg_SavingAborted();

		void sg_PasswordRequired(QSemaphore*, QString*, bool);
		void sg_ConfirmationRequest(QSemaphore*, QString, bool*);
		void sg_Message(QString);


	private slots:
		void sl_Folder_ItemAdded(AbstractFolderItem* const, int);
		void sl_Folder_ItemRemoved(AbstractFolderItem* const);
		void sl_ItemDataChanged(); // when folder or note or tag were changed

		void sl_Note_TagAdded(Tag*);
		void sl_Note_TagRemoved(Tag*);

		void sl_returnSelfToMainThread();
		void sl_InitCustomIcons();

	};
}

#endif // DOCUMENT_H
