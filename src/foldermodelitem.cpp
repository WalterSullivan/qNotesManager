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

#include <QBrush>
#include <QApplication>
#include <QPalette>
#include <QPainter>

using namespace qNotesManager;

FolderModelItem::FolderModelItem(Folder* folder) :
		BaseModelItem(BaseModelItem::folder),
		folder(folder) {
	Q_ASSERT(folder != 0);
	QObject::connect(folder, SIGNAL(sg_VisualPropertiesChanged()),
					 this, SLOT(sl_Folder_PropertiesChanged()));
}

QVariant FolderModelItem::data(int role) const {
	if (role == Qt::DecorationRole) {
		if (folder->IsLocked()) {
			QPixmap shadedIcon = QIcon(folder->GetIcon()).pixmap(folder->GetIcon().size(),
													   QIcon::Disabled, QIcon::On);
			drawLockedIcon(shadedIcon); // TODO: cache 'locked' icon
			return shadedIcon;

		} else {
			return folder->GetIcon();
		}
	} else if (role == Qt::DisplayRole) {
		QString childrenCount = QString(" (%1)").arg(QString::number(folder->Items.Count()));
		QString returnValue = folder->GetName();
		if (Application::I()->Settings.showNumberOfItemsInParentItemTitle) {
			returnValue.append(childrenCount);
		}
		return returnValue;
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

Folder* FolderModelItem::GetStoredData() const {
	return folder;
}

void FolderModelItem::sl_Folder_PropertiesChanged() {
	emit sg_DataChanged(this);
}

// virtual
bool FolderModelItem::LessThan(const BaseModelItem* item) const {
	if (item->DataType() != BaseModelItem::folder) {
		return BaseModelItem::LessThan(item);
	}
	const FolderModelItem* folderItem = dynamic_cast<const FolderModelItem*>(item);
	return folder->GetName() < folderItem->folder->GetName();
}

void FolderModelItem::drawLockedIcon(QPixmap& pixmap) const {
	QPixmap lock = QPixmap(":/gui/lock-small");

	QPainter painter;
	painter.begin(&pixmap);
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.drawPixmap(QRect(0, 0, 16, 16), lock);
	painter.end();
}
