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

#include "foldermodelitem.h"

#include "folder.h"
#include "application.h"
#include "global.h"

#include <QBrush>
#include <QApplication>
#include <QPalette>
#include <QPainter>

using namespace qNotesManager;

FolderModelItem::FolderModelItem(Folder* _folder) :
		BaseModelItem(BaseModelItem::folder),
		folder(_folder) {
	cachedItemPixmapKey = 0;
	if (folder == nullptr) {
		CRITICAL("Null pointer recieved");
	} else {
		QObject::connect(folder, SIGNAL(sg_VisualPropertiesChanged()),
					 this, SLOT(sl_Folder_PropertiesChanged()));
	}
}

QVariant FolderModelItem::data(int role) const {
	if (folder == nullptr) {
		return QVariant();
	}
	if (role == Qt::DecorationRole) {
		if (cachedItemPixmapKey != folder->GetIcon().cacheKey()) {
			drawLockedIcon();
		}
		if (folder->IsLocked()) {
			return cachedLockedItemPixmap;
		} else {
			return folder->GetIcon();
		}
	} else if (role == Qt::DisplayRole) {
		QString childrenCount = QString(" (%1)").arg(QString::number(folder->Items.Count()));
		QString returnValue = folder->GetName();
		if (Application::I()->Settings.GetShowNumberOfItems()) {
			if (
					(folder->Items.Count() > 0) ||
					((folder->Items.Count() == 0) && (Application::I()->Settings.GetShowZeroChildren()))
				) {
				returnValue.append(childrenCount);
			}
		}
		return returnValue;
	} else if (role == Qt::ToolTipRole) {
		return folder->GetName();
	} else if (role == Qt::EditRole) {
		return folder->GetName();
	} else if (role == Qt::BackgroundRole) {
		if (folder->IsLocked()) {
			return QBrush();
		} else {
			return QBrush(folder->GetNameBackColor());
		}
	} else if (role == Qt::ForegroundRole) {
		if (folder->IsLocked()) {
			return QBrush(QApplication::palette().color(QPalette::Disabled, QPalette::WindowText));
		} else {
			return QBrush(folder->GetNameForeColor());
		}
	} else {
		return QVariant();
	}
}

bool FolderModelItem::setData(const QVariant& value, int role) {
	if (!value.isValid()) {return false;}

	if (role == Qt::EditRole) {
		QString newName = value.toString();
		newName.replace(QRegExp("[\a\e\f\n\r\t\v]"), " ");
		folder->SetName(newName);
		return true;
	}
	return false;
}

/* virtual */
Qt::ItemFlags FolderModelItem::flags () const {
	Qt::ItemFlags flags = BaseModelItem::flags();

	if (!folder->IsLocked() && folder->GetType() == Folder::UserFolder) {
		return flags | Qt::ItemIsEditable;
	}

	return flags;
}

Folder* FolderModelItem::GetStoredData() const {
	return folder;
}

void FolderModelItem::sl_Folder_PropertiesChanged() {
	if (cachedItemPixmapKey != folder->GetIcon().cacheKey()) {
		drawLockedIcon();
	}
	emit sg_DataChanged(this);
}

// virtual
bool FolderModelItem::LessThan(const BaseModelItem* item) const {
	if (item->DataType() != BaseModelItem::folder) {
		return BaseModelItem::LessThan(item);
	}
	const FolderModelItem* folderItem = dynamic_cast<const FolderModelItem*>(item);
	return folder->GetName().toUpper() < folderItem->folder->GetName().toUpper();
}

void FolderModelItem::drawLockedIcon() const {
	cachedItemPixmapKey = folder->GetIcon().cacheKey();
	cachedLockedItemPixmap = QIcon(folder->GetIcon()).pixmap(folder->GetIcon().size(),
											   QIcon::Disabled, QIcon::On);
	if (cachedLockedItemPixmap.isNull()) {return;}

	QPixmap lock = QPixmap(":/gui/lock-small");

	QPainter painter;
	painter.begin(&cachedLockedItemPixmap);
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.drawPixmap(QRect(0, 0, 16, 16), lock);
	painter.end();
}
