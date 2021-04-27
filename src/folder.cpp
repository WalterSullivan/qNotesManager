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

#include "folder.h"

#include "application.h"
#include "document.h"
#include "note.h"
#include "global.h"

#include <QDebug>

using namespace qNotesManager;

Folder::Folder(QString _name, FolderType _type) :
		AbstractFolderItem(AbstractFolderItem::Type_Folder),
		type(_type),
		name(_name),
		defaultForeColor(QColor(0, 0, 0, 255)),
		defaultBackColor(QColor(0, 0, 0, 0)),
		nameForeColor(defaultForeColor),
		nameBackColor(defaultBackColor),
		locked(false),
		iconID(QString()),
		creationDate(QDateTime::currentDateTime()),
		modificationDate(QDateTime::currentDateTime()),
		expanded(false),
		Items(this) {
	QObject::connect(&Items, SIGNAL(sg_ItemAdded(AbstractFolderItem* const, int)),
					   this, SIGNAL(sg_ItemAdded(AbstractFolderItem* const, int)));
	QObject::connect(&Items, SIGNAL(sg_ItemMoved(AbstractFolderItem* const,int, Folder*)),
					   this, SIGNAL(sg_ItemMoved(AbstractFolderItem* const,int, Folder*)));
	QObject::connect(&Items, SIGNAL(sg_ItemRemoved(AbstractFolderItem* const)),
					   this, SIGNAL(sg_ItemRemoved(AbstractFolderItem* const)));
	QObject::connect(&Items, SIGNAL(sg_Cleared()),
					   this, SIGNAL(sg_ItemsCollectionCleared()));
	QObject::connect(&Items, SIGNAL(sg_ItemAboutToBeAdded(AbstractFolderItem*const, int)),
					   this, SIGNAL(sg_ItemAboutToBeAdded(AbstractFolderItem*const, int)));
	QObject::connect(&Items, SIGNAL(sg_ItemAboutToBeMoved(AbstractFolderItem*const,int, Folder*)),
					   this, SIGNAL(sg_ItemAboutToBeMoved(AbstractFolderItem*const,int, Folder*)));
	QObject::connect(&Items, SIGNAL(sg_ItemAboutToBeRemoved(AbstractFolderItem*const)),
					   this, SIGNAL(sg_ItemAboutToBeRemoved(AbstractFolderItem*const)));
	QObject::connect(&Items, SIGNAL(sg_AboutToClear()),
					   this, SIGNAL(sg_ItemsCollectionAboutToClear()));
	QObject::connect(&Items, SIGNAL(sg_ItemAdded(AbstractFolderItem*const,int)),
					 this, SIGNAL(sg_DataChanged()));
	QObject::connect(&Items, SIGNAL(sg_ItemMoved(AbstractFolderItem*const,int,Folder*)),
					 this, SIGNAL(sg_DataChanged()));
	QObject::connect(&Items, SIGNAL(sg_ItemRemoved(AbstractFolderItem*const)),
					 this, SIGNAL(sg_DataChanged()));
	QObject::connect(&Items, SIGNAL(sg_Cleared()),
					 this, SIGNAL(sg_DataChanged()));

	if (type == TempFolder) {
		name = "Temporary";
	} else if (type == TrashFolder) {
		name = "Trash bin";
	}
	if (name.isEmpty()) {name = "New folder";}
}

Folder::~Folder() {
	for (int i = Items.Count() - 1; i >= 0 ; i--) {
		AbstractFolderItem* item = Items.ItemAt(i);
		Items.Remove(item);
		delete item;
	}
}

QString Folder::GetName() const {
	return name;
}

void Folder::SetName (const QString s) {
	if (name == s) {return;}

	name = s;

	emit sg_VisualPropertiesChanged();
	onChange();
}

Folder::FolderType Folder::GetType() const {
	return type;
}

void Folder::SetType(Folder::FolderType _type) {
	type = _type;
}

QPixmap Folder::GetIcon() const {
	if (type == UserFolder) {
		if (Application::I()->CurrentDocument() == nullptr) {
			return QPixmap();
		}
		return Application::I()->CurrentDocument()->GetItemIcon(iconID);
	} else if (type == TrashFolder) {
		if (Items.Count() > 0) {
			return QPixmap(":/gui/bin-full");
		} else {
			return QPixmap(":/gui/bin");
		}
	} else if (type == TempFolder) {
		return QPixmap(":/gui/folder-excl");
	}
	return QPixmap();
}

void Folder::SetIconID(const QString id) {
	if (type != UserFolder) {return;}
	if (iconID == id) {return;}
	if (id.isEmpty()) {
		WARNING("New icon id is empty");
		return;
	}

	iconID = id;

	emit sg_VisualPropertiesChanged();
	onChange();
}

QString Folder::GetIconID() const {
	return iconID;
}

QDateTime Folder::GetCreationDate() const {
	return creationDate;
}

QDateTime Folder::GetModificationDate() const {
	return modificationDate;
}

QColor Folder::GetNameForeColor() const {
	return nameForeColor;
}

void Folder::SetNameForeColor(const QColor c) {
	if (!c.isValid()) {return;}
	if (nameForeColor == c) {return;}

	nameForeColor = c;

	emit sg_VisualPropertiesChanged();
	onChange();
}

QColor Folder::GetNameBackColor() const {
	return nameBackColor;
}

void Folder::SetNameBackColor(const QColor c) {
	if (!c.isValid()) {return;}
	if (nameBackColor == c) {return;}

	nameBackColor = c;

	emit sg_VisualPropertiesChanged();
	onChange();
}

QColor Folder::GetDefaultForeColor() const {
	return defaultForeColor;
}

QColor Folder::GetDefaultBackColor() const {
	return defaultBackColor;
}

bool Folder::IsLocked() const {
	return locked;
}

void Folder::SetLocked(bool e) {
	if (locked == e) {return;}

	locked = e;

	if (!Application::I()->CurrentDocument()->LockFolderItems) {return;}

	for (int i = 0; i < Items.Count(); i++) {
		AbstractFolderItem* item = Items.ItemAt(i);
		if (item->GetItemType() == AbstractFolderItem::Type_Folder) {
			Folder* f = dynamic_cast<Folder*>(item);
			f->SetLocked(e);
		} else if (item->GetItemType() == AbstractFolderItem::Type_Note) {
			Note* n = dynamic_cast<Note*>(item);
			n->SetLocked(e);
		} else {
			WARNING("Unknown item type");
		}
	}

	emit sg_VisualPropertiesChanged();
	onChange();
}

bool Folder::IsExpanded() const {
	return expanded;
}

void Folder::SetExpanded(bool value) {
	expanded = value;
}

QString Folder::GetPath() const {
	QString path = name;

	Folder* f = GetParent();
	while(f != nullptr && f->GetParent() != nullptr) {
		path.prepend(f->name + "/");
		f = f->GetParent();
	}

	return path;
}

void Folder::onChange() {
	modificationDate = QDateTime::currentDateTime();

	emit sg_DataChanged();
}
