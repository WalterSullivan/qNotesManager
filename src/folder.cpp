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
#include "boibuffer.h"
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
	if (type == UserFolder) {
		//iconID = Application::I()->CurrentDocument()->DefaultFolderIcon;
		iconID = Application::I()->DefaultFolderIcon;
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
	modificationDate = QDateTime::currentDateTime();

	emit sg_VisualPropertiesChanged();
	emit sg_DataChanged();
}

Folder::FolderType Folder::GetType() const {
	return type;
}

void Folder::SetType(Folder::FolderType _type) {
	type = _type;
}

QPixmap Folder::GetIcon() const {
	if (type == UserFolder) {
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
	modificationDate = QDateTime::currentDateTime();

	emit sg_VisualPropertiesChanged();
	emit sg_DataChanged();
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
	modificationDate = QDateTime::currentDateTime();

	emit sg_VisualPropertiesChanged();
}

QColor Folder::GetNameBackColor() const {
	return nameBackColor;
}

void Folder::SetNameBackColor(const QColor c) {
	if (!c.isValid()) {return;}
	if (nameBackColor == c) {return;}

	nameBackColor = c;
	modificationDate = QDateTime::currentDateTime();

	emit sg_VisualPropertiesChanged();
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
	modificationDate = QDateTime::currentDateTime();

	emit sg_VisualPropertiesChanged();

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
}

QString Folder::GetPath() const {
	//if (GetParent() == 0) {return "";}

	QString path = name;

	Folder* f = GetParent();
	while(f != 0 && f->GetParent() != 0) {
		path.prepend(f->name + "/");
		f = f->GetParent();
	}

	return path;
}

void Folder::Serialize(const int version, BOIBuffer& stream) const {
	(void)version;
	const QByteArray s_caption = name.toUtf8();
	const quint32 s_captionSize = s_caption.size();
	const quint32 s_creationDate = creationDate.toTime_t();
	const quint32 s_modificationDate = modificationDate.toTime_t();
	const QByteArray s_iconID = iconID.toAscii();
	const quint32 s_iconIDSize = s_iconID.size();
	const quint32 s_backColor = nameBackColor.rgba();
	const quint32 s_foreColor = nameForeColor.rgba();
	const quint8 s_locked = (quint8)locked;
	const quint32 s_itemSize =	s_captionSize +
								sizeof(s_captionSize) +
								sizeof(s_creationDate) +
								sizeof(s_modificationDate) +
								s_iconIDSize +
								sizeof(s_iconIDSize) +
								sizeof(s_backColor) +
								sizeof(s_foreColor) +
								sizeof(s_locked);

	stream.write(s_itemSize);
	stream.write(s_captionSize);
	stream.write(s_caption.constData(), s_captionSize);
	stream.write(s_creationDate);
	stream.write(s_modificationDate);
	stream.write(s_iconIDSize);
	stream.write(s_iconID.constData(), s_iconIDSize);
	stream.write(s_backColor);
	stream.write(s_foreColor);
	stream.write(s_locked);
}

/* static */
Folder* Folder::Deserialize(const int version, BOIBuffer& stream) {
	(void)version;
	qint64 bytesRead = 0;

	quint32 r_itemSize = 0;
	bytesRead = stream.read(r_itemSize);

	const qint64 streamStartPos = stream.pos();

	quint32 r_captionSize = 0;
	bytesRead = stream.read(r_captionSize);
	QByteArray r_caption(r_captionSize, 0x0);
	bytesRead = stream.read(r_caption.data(), r_captionSize);
	quint32 r_creationDate = 0;
	bytesRead = stream.read(r_creationDate);
	quint32 r_modificationDate = 0;
	bytesRead = stream.read(r_modificationDate);
	quint32 r_iconIDSize = 0;
	bytesRead = stream.read(r_iconIDSize);
	QByteArray r_iconID(r_iconIDSize, 0x0);
	bytesRead = stream.read(r_iconID.data(), r_iconIDSize);
	quint32 r_backColor = 0;
	bytesRead = stream.read(r_backColor);
	quint32 r_foreColor = 0;
	bytesRead = stream.read(r_foreColor);
	quint8 r_locked = 0;
	bytesRead = stream.read(r_locked);

	const quint32 bytesToSkip = r_itemSize - (stream.pos() - streamStartPos);

	if (bytesToSkip != 0) {
		stream.seek(stream.pos() + bytesToSkip); // If chunck has more data in case of newer file version.
	}

	Folder* f = new Folder("");
	f->name = r_caption;
	f->nameForeColor.setRgba(r_foreColor);
	f->nameBackColor.setRgba(r_backColor);
	f->locked = (bool)r_locked;
	f->iconID = r_iconID;
	f->creationDate = QDateTime::fromTime_t(r_creationDate);
	f->modificationDate = QDateTime::fromTime_t(r_modificationDate);

	return f;
}
