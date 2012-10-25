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

#include "tagmodelitem.h"

#include "tag.h"
#include "application.h"

#include <QPixmap>

using namespace qNotesManager;

TagModelItem::TagModelItem(const Tag* tag) : BaseModelItem(BaseModelItem::tag), _tag(tag) {
	Q_ASSERT(tag != 0);
}

QVariant TagModelItem::data(int role) const {
	if (role == Qt::DecorationRole) {
		return QPixmap(":/standard/tag");
	} else if (role == Qt::DisplayRole) {
		QString childrenCount = QString(" (%1)").arg(QString::number(_tag->Owners.Count()));
		QString returnValue = _tag->GetName();
		if (Application::I()->Settings.showNumberOfItemsInParentItemTitle) {
			returnValue.append(childrenCount);
		}
		return returnValue;
	} else {
		return QVariant();
	}
}

const Tag* TagModelItem::GetTag() const {
	return _tag;
}

// virtual
bool TagModelItem::LessThan(const BaseModelItem* item) const {
	if (item->DataType() != BaseModelItem::tag) {
		return BaseModelItem::LessThan(item);
	}
	const TagModelItem* tagItem = dynamic_cast<const TagModelItem*>(item);
	return _tag->GetName() < tagItem->_tag->GetName();
}
