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

#include "document.h"

#include "folder.h"
#include "abstractfolderitem.h"
#include "note.h"
#include "tag.h"
#include "application.h"
#include "hierarchymodel.h"
#include "tagsmodel.h"
#include "datesmodel.h"
#include "cachedimagefile.h"
#include "serializer.h"
#include "global.h"
#include "compressor.h"

#include <QFile>
#include <QFileInfo>
#include <QBuffer>
#include <QDebug>
#include <QInputDialog>
#include <QThread>
#include <QCoreApplication>


using namespace qNotesManager;

Document::Document() : QObject(0) {
	inInitMode = false;

	rootFolder = new Folder("_root_", Folder::SystemFolder);
	rootFolder->setParent(this);
	QObject::connect(rootFolder, SIGNAL(sg_ItemAdded(AbstractFolderItem*const, int)),
					 this, SLOT(sl_Folder_ItemAdded(AbstractFolderItem* const, int)));
	QObject::connect(rootFolder, SIGNAL(sg_ItemRemoved(AbstractFolderItem*const)),
					 this, SLOT(sl_Folder_ItemRemoved(AbstractFolderItem*const)));
	QObject::connect(rootFolder, SIGNAL(sg_DataChanged()), this, SLOT(sl_ItemDataChanged()));

	tempFolder = new Folder("Temporary", Folder::TempFolder);
	tempFolder->setParent(this);
	QObject::connect(tempFolder, SIGNAL(sg_ItemAdded(AbstractFolderItem*const, int)),
					 this, SLOT(sl_Folder_ItemAdded(AbstractFolderItem* const, int)));
	QObject::connect(tempFolder, SIGNAL(sg_ItemRemoved(AbstractFolderItem*const)),
					 this, SLOT(sl_Folder_ItemRemoved(AbstractFolderItem*const)));
	QObject::connect(tempFolder, SIGNAL(sg_DataChanged()), this, SLOT(sl_ItemDataChanged()));

	trashFolder = new Folder("Trash", Folder::TrashFolder);
	trashFolder->setParent(this);
	QObject::connect(trashFolder, SIGNAL(sg_ItemAdded(AbstractFolderItem*const, int)),
					 this, SLOT(sl_Folder_ItemAdded(AbstractFolderItem* const, int)));
	QObject::connect(trashFolder, SIGNAL(sg_ItemRemoved(AbstractFolderItem*const)),
					 this, SLOT(sl_Folder_ItemRemoved(AbstractFolderItem*const)));
	QObject::connect(trashFolder, SIGNAL(sg_DataChanged()), this, SLOT(sl_ItemDataChanged()));

	pinnedFolder = 0;

	tagsListModel = new QStandardItemModel(this);

	hierarchyModel = new HierarchyModel(this);

	tagsModel = new TagsModel(this);

	creationDateModel = new DatesModel(DatesModel::CreationDate, this);

	modificationDateModel = new DatesModel(DatesModel::ModifyDate, this);

	textDateModel = new DatesModel(DatesModel::TextDate, this);


	LockFolderItems = true;


	fileName = QString();
	fileVersion = Serializer::actualSpecificationVersion;
	compressionLevel = Compressor::MaximumLevel;
	cipherID = 0;
	password = QByteArray();
	creationDate = QDateTime::currentDateTime();
	modificationDate = QDateTime::currentDateTime();
	hasUnsavedData = false;
	isModified = false;


	DefaultFolderIcon = Application::I()->DefaultFolderIcon;
	DefaultNoteIcon = Application::I()->DefaultNoteIcon;

	tagIcon = QIcon(":/gui/tag");

	fileTimeStamp = QDateTime::currentDateTime();
	doNotReloadFlag = false;
}

Document::~Document()
{
    password.fill('z');

	delete rootFolder;
	delete trashFolder;
	delete tempFolder;

	// Remove custom icons from model
	Application::I()->GetIconsModel()->removeRows(
			Application::I()->GetStandardIconsCount(),
			customIcons.count());

	foreach (QString key, customIcons.keys()) {
		delete customIcons[key];
	}

	foreach (Tag* t, allTags) {
		delete t;
	}
}

QString Document::GetFilename() const {
	return fileName;
}

QDateTime Document::GetCreationDate() const {
	return creationDate;
}

QDateTime Document::GetModificationDate() const {
	return modificationDate;
}

Folder* Document::GetTempFolder() const {
	return tempFolder;
}

Folder* Document::GetTrashFolder() const {
	return trashFolder;
}

Folder* Document::GetRoot() const {
	return rootFolder;
}

Folder* Document::GetPinnedFolder() const {
	return pinnedFolder;
}

void Document::SetPinnedFolder(Folder* newPinnedFolder) {
	pinnedFolder = newPinnedFolder;
	hierarchyModel->SetPinnedFolder(pinnedFolder);
	onChange();
}

void Document::onChange() {
	if (inInitMode) {return;}

	modificationDate = QDateTime::currentDateTime();
	if (!isModified) {isModified = true;}

	if (!hasUnsavedData) {
		hasUnsavedData = true;
		emit sg_Changed();
	}
}

bool Document::HasUnsavedData() const {
	return hasUnsavedData;
}

void Document::Open(QString fileName) {
	Serializer* w = new Serializer();
	QThread* t = new QThread();
	w->moveToThread(t);
	this->moveToThread(t);

	QObject::connect(w, SIGNAL(sg_LoadingAborted()), this, SIGNAL(sg_LoadingAborted()));
	QObject::connect(w, SIGNAL(sg_LoadingFailed(QString)), this, SIGNAL(sg_LoadingFailed(QString)));
	QObject::connect(w, SIGNAL(sg_LoadingFinished()), this, SIGNAL(sg_LoadingFinished()));
	QObject::connect(w, SIGNAL(sg_LoadingProgress(int)), this, SIGNAL(sg_LoadingProgress(int)));
	QObject::connect(w, SIGNAL(sg_LoadingStarted()), this, SIGNAL(sg_LoadingStarted()));
	QObject::connect(w, SIGNAL(sg_ConfirmationRequest(QSemaphore*,QString,bool*)), this, SIGNAL(sg_ConfirmationRequest(QSemaphore*,QString,bool*)));
	QObject::connect(w, SIGNAL(sg_PasswordRequired(QSemaphore*,QString*, bool)), this, SIGNAL(sg_PasswordRequired(QSemaphore*,QString*, bool)));
	QObject::connect(w, SIGNAL(sg_Message(QString)), this, SIGNAL(sg_Message(QString)));

	QObject::connect(t, SIGNAL(started()), w, SLOT(sl_start()));
	QObject::connect(w, SIGNAL(sg_finished()), t, SLOT(quit()));
	QObject::connect(w, SIGNAL(sg_finished()), w, SLOT(deleteLater()));
	QObject::connect(w, SIGNAL(sg_finished()), t, SLOT(deleteLater()));
	QObject::connect(w, SIGNAL(sg_finished()), this, SLOT(sl_returnSelfToMainThread()), Qt::DirectConnection);
	QObject::connect(w, SIGNAL(sg_finished()), this, SLOT(sl_InitCustomIcons()));

	w->Load(this, fileName);
	t->start();
}

void Document::Save(QString name, quint16 version) {
	fileVersion = Serializer::actualSpecificationVersion;

	if (name.isEmpty() && this->fileName.isEmpty()) {
		WARNING("No filename specified");
		return;
	}

	Serializer* w = new Serializer();
    QThread *t = new QThread(this);
	w->moveToThread(t);
	this->moveToThread(t);

	QObject::connect(w, SIGNAL(sg_SavingAborted()), this, SIGNAL(sg_SavingAborted()));
	QObject::connect(w, SIGNAL(sg_SavingFailed(QString)), this, SIGNAL(sg_SavingFailed(QString)));
	QObject::connect(w, SIGNAL(sg_SavingFinished()), this, SIGNAL(sg_SavingFinished()));
	QObject::connect(w, SIGNAL(sg_SavingProgress(int)), this, SIGNAL(sg_SavingProgress(int)));
	QObject::connect(w, SIGNAL(sg_SavingStarted()), this, SIGNAL(sg_SavingStarted()));
	QObject::connect(w, SIGNAL(sg_ConfirmationRequest(QSemaphore*,QString,bool*)), this,
                     SIGNAL(sg_ConfirmationRequest(QSemaphore*,QString,bool*)));
	QObject::connect(w, SIGNAL(sg_Message(QString)), this, SIGNAL(sg_Message(QString)));

	QObject::connect(t, SIGNAL(started()), w, SLOT(sl_start()));
    QObject::connect(w, SIGNAL(sg_finished()), this, SLOT(sl_returnSelfToMainThread()),
                     Qt::DirectConnection);

    QObject::connect(w, SIGNAL(sg_finished()), w, SLOT(deleteLater()));
    QObject::connect(w, SIGNAL(sg_finished()), t, SLOT(quit()));
    QObject::connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));

	quint16 newVersion = version > 0 ? version : this->fileVersion;
	QString newName = name.isEmpty() ? this->fileName : name;

	w->Save(this, newName, newVersion);
	t->start();

	emit sg_Changed();
}

void Document::sl_returnSelfToMainThread() {
	// Called from different thread after loading
	this->moveToThread(QCoreApplication::instance()->thread());
}

void Document::sl_InitCustomIcons() {
	foreach (QString name, customIcons.keys()) {
		CachedImageFile* image = customIcons[name];

		QStandardItem* i = new QStandardItem(image->GetPixmap(QSize(16, 16)), QString());
		i->setData(name, Qt::UserRole + 1);
		i->setData("Custom icons", Qt::UserRole + 2);
		i->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		Application::I()->GetIconsModel()->appendRow(i);
	}
}

void Document::sl_Folder_ItemAdded(AbstractFolderItem* const item, int) {
	RegisterItem(item);
	onChange();
}

void Document::sl_Folder_ItemRemoved(AbstractFolderItem* const item) {
	UnregisterItem(item);
	onChange();
}

void Document::RegisterItem(AbstractFolderItem* const item) {
	if (item->GetItemType() == AbstractFolderItem::Type_Folder) {
		Folder* f = dynamic_cast<Folder*>(item);

		f->setParent(this); // QObject parentship

		if (f->GetIconID().isEmpty()) {
			f->SetIconID(DefaultFolderIcon);
		}

		QObject::connect(f, SIGNAL(sg_ItemAdded(AbstractFolderItem*const, int)),
						 this, SLOT(sl_Folder_ItemAdded(AbstractFolderItem*const, int)));
		QObject::connect(f, SIGNAL(sg_ItemRemoved(AbstractFolderItem*const)),
						 this, SLOT(sl_Folder_ItemRemoved(AbstractFolderItem*const)));

		QObject::connect(f, SIGNAL(sg_DataChanged()), this, SLOT(sl_ItemDataChanged()));

		allFolders.append(f);
		emit sg_ItemRegistered(f);

		for (int i = 0; i < f->Items.Count(); ++i) {
			RegisterItem(f->Items.ItemAt(i));
		}

	} else if (item->GetItemType() == AbstractFolderItem::Type_Note) {
		Note* n = dynamic_cast<Note*>(item);

		n->setParent(this);  // QObject parentship

		if (n->GetIconID().isEmpty()) {
			n->SetIconID(DefaultNoteIcon);
		}
		QObject::connect(n, SIGNAL(sg_DataChanged()), this, SLOT(sl_ItemDataChanged()));
		QObject::connect(n, SIGNAL(sg_TagAdded(Tag*)), this, SLOT(sl_Note_TagAdded(Tag*)));
		QObject::connect(n, SIGNAL(sg_TagRemoved(Tag*)), this, SLOT(sl_Note_TagRemoved(Tag*)));

		// TODO: Register tags ?

		allNotes.append(n);
		emit sg_ItemRegistered(n);
	}
}

void Document::UnregisterItem(AbstractFolderItem* const item) {
	if (item->GetItemType() == AbstractFolderItem::Type_Folder) {
		Folder* f = dynamic_cast<Folder*>(item);
		QObject::disconnect(f, 0, this, 0);

		allFolders.removeAll(f);
		emit sg_ItemUnregistered(f);

		for (int i = 0; i < f->Items.Count(); ++i) {
			UnregisterItem(f->Items.ItemAt(i));
		}

	} else if (item->GetItemType() == AbstractFolderItem::Type_Note) {
		Note* n = dynamic_cast<Note*>(item);
		QObject::disconnect(n, 0, this, 0);

		n->Tags.Clear(); // ? Tags must be unregistered, but this line modifies note.

		allNotes.removeAll(n);

		if (bookmarks.contains(n)) {
			RemoveBookmark(n);
		}

		emit sg_ItemUnregistered(n);
	}
}

void Document::RegisterTag(Tag* tag) {
	allTags.append(tag);
	tag->setParent(this);
	tagsByName.insert(tag->GetName(), tag);
	QStandardItem* i = new QStandardItem(tagIcon, tag->GetName());
	tagsListModel->appendRow(i);
	tagsListModel->sort(0, Qt::AscendingOrder);
	emit sg_ItemRegistered(tag);
}

void Document::UnregisterTag(Tag* tag) {
	allTags.removeAll(tag);
	tag->setParent(0);
	tagsByName.remove(tag->GetName());
	QList<QStandardItem*> itemsList = tagsListModel->findItems(tag->GetName(), Qt::MatchExactly, 0);
	QStandardItem* item = tagsListModel->takeItem(itemsList.at(0)->row());
	delete item;

	emit sg_ItemUnregistered(tag);
	delete tag;
}

void Document::sl_Note_TagAdded(Tag* tag) {
	if (!tag) {
		WARNING("Null pointer recieved");
		return;
	}

	if (!allTags.contains(tag)) {
		RegisterTag(tag);
	}
}

void Document::sl_Note_TagRemoved(Tag* tag) {
	if (!tag) {
		WARNING("Null pointer recieved");
		return;
	}
	if (!allTags.contains(tag)) {
		WARNING("Unknown tag");
	}

	if (tag->Owners.Count() == 0) {
		UnregisterTag(tag);
	}
}

// when folder or note or tag were changed
void Document::sl_ItemDataChanged() {
	QObject* sender = QObject::sender();
	AbstractFolderItem* item = dynamic_cast<AbstractFolderItem*>(sender);
	if (item != 0) {
		if (item->GetItemType() == AbstractFolderItem::Type_Note) {
			Note* n = dynamic_cast<Note*>(item);
			if (n != 0 && bookmarks.contains(n)) {
				emit sg_BookmarksListChanged();
			}
		}
	}

	onChange();
}

Tag* Document::FindTagByName(QString name) const {
	return tagsByName.contains(name) ? tagsByName.value(name) : 0;
}

QAbstractItemModel* Document::GetTagsListModel() const {
	return tagsListModel;
}

HierarchyModel* Document::GetHierarchyModel() const {
	return hierarchyModel;
}

TagsModel* Document::GetTagsModel() const {
	return tagsModel;
}

DatesModel* Document::GetCreationDatesModel() const {
	return creationDateModel;
}

DatesModel* Document::GetModificationDatesModel() const {
	return modificationDateModel;
}

DatesModel* Document::GetTextDatesModel() const {
	return textDateModel;
}

QList<Tag*> Document::GetTagsList() const {
	return allTags;
}

QList<Note*> Document::GetNotesList() const {
	return allNotes;
}

void Document::AddCustomIcon(CachedImageFile* image) {
	if (!image) {
		WARNING("Null image recieved");
		return;
	}

	if (!image->IsValidImage()) {
		WARNING("Invalid image recieved");
		delete image;
		return;
	}

	QString name = image->GetMD5();
	customIcons.insert(name, image);

	QStandardItem* i = new QStandardItem(image->GetPixmap(QSize(16, 16)), QString());
	i->setData(name, Qt::UserRole + 1);
	i->setData("Custom icons", Qt::UserRole + 2);
	i->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	Application::I()->GetIconsModel()->appendRow(i);

	onChange();
}

void Document::AddCustomIconToStorage(CachedImageFile* image) {
	if (!image) {
		WARNING("Null image recieved");
		return;
	}

	if (!image->IsValidImage()) {
		WARNING("Invalid image recieved");
		delete image;
		return;
	}

	QString name = image->GetMD5();
	customIcons.insert(name, image);
}

void Document::RemoveCustomIcon(QString key) {
	if (!customIcons.contains(key)) {
		WARNING("Icon with passed key not found");
		return;
	}

	if (DefaultFolderIcon == key) {
		DefaultFolderIcon = Application::I()->DefaultFolderIcon;
	}

	if (DefaultNoteIcon == key) {
		DefaultNoteIcon = Application::I()->DefaultNoteIcon;
	}

	QList<QStandardItem*> itemsList = Application::I()->GetIconsModel()->findItems(key, Qt::MatchExactly, 0);
	if (itemsList.size() != 1) {
		WARNING("A few icons with the same key found");
	}
	QStandardItem* item = Application::I()->GetIconsModel()->takeItem(itemsList.at(0)->row());
	if (item == 0) {
		WARNING("Error retrieving icon model item");
		return;
	}


	for (int i = 0; i < allNotes.size(); ++i) {
		if (allNotes.at(i)->GetIconID() == key) {
			allNotes.at(i)->SetIconID(DefaultNoteIcon);
		}
	}

	for (int i = 0; i < allFolders.size(); ++i) {
		if (allFolders.at(i)->GetIconID() == key) {
			allFolders.at(i)->SetIconID(DefaultFolderIcon);
		}
	}

	delete item;

	delete customIcons[key];
	customIcons.remove(key);

	onChange();
}

QPixmap Document::GetItemIcon(const QString key) const {
	if (key.isEmpty()) {
		WARNING("Empty key passed");
		return QPixmap();
	}

	if (key.mid(0, 1) == ":") {
		QPixmap p = Application::I()->GetStandardIcon(key);
		if (p.isNull()) {
			WARNING(qPrintable("Icon with passed key not found: " + key));
		}
		return p;
	} else {
		if (customIcons.contains(key)) {
			return customIcons.value(key)->GetPixmap(QSize(16, 16));
		} else {
			WARNING(qPrintable("Icon with passed key not found: " + key));
			return QPixmap();
		}
	}
}

quint8 Document::GetCompressionLevel() const {
	return compressionLevel;
}

void Document::SetCompressionLevel(const quint8 level) {
	if (level < Compressor::MinimumLevel || level > Compressor::MaximumLevel) {
		WARNING("Wrong argument value recieved");
	}

	if (compressionLevel != level) {
		compressionLevel = level;
		onChange();
	}
}

quint8 Document::GetCipherID() const {
	return cipherID;
}

QString Document::GetPassword() const {
	return QString(password);
}

void Document::SetCipherData(const quint8 id, const QString& _password) {
	if (id != 0 && _password.isEmpty()) {
		WARNING("Password is empty");
		return;
	}

	if (cipherID != id) {
		cipherID = id;
		onChange();
	}

	if (password != _password) {
		if (cipherID == 0) {
			password = QByteArray();
		} else {
			password = _password.toLocal8Bit();
		}
		onChange();
	}
}

QString Document::GetDefaultNoteIcon() const {
	return DefaultNoteIcon;
}

QString Document::GetDefaultFolderIcon() const {
	return DefaultFolderIcon;
}

void Document::SetDefaultNoteIcon(QString id) {
	if (DefaultNoteIcon == id) {return;}

	DefaultNoteIcon = id;
	onChange();
}

void Document::SetDefaultFolderIcon(QString id) {
	if (DefaultFolderIcon == id) {return;}

	DefaultFolderIcon = id;
	onChange();
}

int Document::GetBookmarksCount() const {
	return bookmarks.count();
}

Note* Document::GetBookmark(int index) const {
	if (index < 0 || index >= bookmarks.count()) {return 0;}

	return bookmarks[index];
}

void Document::AddBookmark(Note* note) {
	if (bookmarks.contains(note)) {return;}

	bookmarks.push_back(note);
	onChange();
	emit sg_BookmarksListChanged();
}

void Document::RemoveBookmark(Note* note) {
	if (!bookmarks.contains(note)) {return;}

	bookmarks.removeOne(note);
	onChange();
	emit sg_BookmarksListChanged();
}

bool Document::IsBookmark(Note* note) {
	if (note == 0) {return false;}

	return bookmarks.contains(note);
}

QDateTime Document::GetFileTimeStamp() const {
	return fileTimeStamp;
}

bool Document::DoNotReload() const {
	return doNotReloadFlag;
}
void Document::SetDoNotReload(bool v) const {
	doNotReloadFlag = v;
}


