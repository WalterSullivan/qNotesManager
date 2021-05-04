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

#include "notemodelitem.h"

#include "basemodel.h"
#include "note.h"
#include "global.h"

#include <QBrush>
#include <QApplication>
#include <QPalette>
#include <QPainter>

using namespace qNotesManager;

NoteModelItem::NoteModelItem(Note* _note) : BaseModelItem(BaseModelItem::note), note(_note) {
	cachedItemPixmapKey = 0;
	if (note == nullptr) {
		WARNING("Null pointer recieved");
	} else {
		QObject::connect(note, SIGNAL(sg_VisualPropertiesChanged()),
					 this, SLOT(sl_Note_PropertiesChanged()));
	}
	IsEditable = true;
}

QVariant NoteModelItem::data(int role) const {
	if (note == nullptr) {
		return QVariant();
	}

	if (role == Qt::DecorationRole) {
		if (cachedItemPixmapKey != note->GetIcon().cacheKey()) {
			drawLockedIcon();
		}
		if (note->IsLocked()) {
			return cachedLockedItemPixmap;
		} else {
			return note->GetIcon();
		}
	} else if (role == Qt::DisplayRole) {
		return note->GetName();
	} else if (role == Qt::ToolTipRole) {
		return note->GetName();
	} else if (role == Qt::EditRole) {
		return note->GetName();
	} else if (role == Qt::BackgroundRole) {
		if (note->IsLocked()) {
			return QBrush();
		} else {
			return QBrush(note->GetNameBackColor());
		}
	} else if (role == Qt::ForegroundRole) {
		if (note->IsLocked()) {
			return QBrush(QApplication::palette().color(QPalette::Disabled, QPalette::WindowText));
		} else {
			return QBrush(note->GetNameForeColor());
		}
	} else {
		return QVariant();
	}
}

bool NoteModelItem::setData(const QVariant& value, int role) {
	if (!value.isValid()) {return false;}

	if (role == Qt::EditRole) {
		QString newName = value.toString();
		newName.replace(QRegExp("[\a\e\f\n\r\t\v]"), " ");
		note->SetName(newName);
		return true;
	}
	return false;
}

/* virtual */
Qt::ItemFlags NoteModelItem::flags () const {
	Qt::ItemFlags flags = BaseModelItem::flags();

	if ((!note->IsLocked()) && (IsEditable)) {
		return flags | Qt::ItemIsEditable;
	}

	return flags;
}

Note* NoteModelItem::GetStoredData() const {
	return note;
}

void NoteModelItem::sl_Note_PropertiesChanged() {
	if (cachedItemPixmapKey != note->GetIcon().cacheKey()) {
		drawLockedIcon();
	}
	emit sg_DataChanged(this);
}

// virtual
bool NoteModelItem::LessThan(const BaseModelItem* item) const {
	if (item->DataType() != BaseModelItem::note) {
		return BaseModelItem::LessThan(item);
	}
	const NoteModelItem* noteItem = dynamic_cast<const NoteModelItem*>(item);
	return note->GetName().toUpper() < noteItem->note->GetName().toUpper();
}

void NoteModelItem::drawLockedIcon() const {
	cachedItemPixmapKey = note->GetIcon().cacheKey();
	cachedLockedItemPixmap = QIcon(note->GetIcon()).pixmap(note->GetIcon().size(),
											   QIcon::Disabled, QIcon::On);
	if (cachedLockedItemPixmap.isNull()) {return;}

	QPixmap lock = QPixmap(":/gui/lock-small");

	QPainter painter;
	painter.begin(&cachedLockedItemPixmap);
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.drawPixmap(QRect(0, 0, 16, 16), lock);
	painter.end();
}
