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

#include "datemodelitem.h"

#include <QDate>
#include <QIcon>

using namespace qNotesManager;

DateModelItem::DateModelItem(DateComponent c, int v) :
		BaseModelItem(BaseModelItem::date),
		value(v),
		component(c) {
	if (v < 0) {v = 0;}
	if (component == Month && v > 11) {v = 11;}
}

// virtual
QVariant DateModelItem::data(int role) const {
	if (role == Qt::DecorationRole) {
		return QIcon(":/gui/date");
	} else if (role == Qt::DisplayRole) {
		switch (component) {
			case Year:
				return QString("%1 (%2)").arg(QString::number(value)).arg(QString::number(ChildrenCount()));
				break;
			case Month:
				{
					QDate month(2000, value, 1);
					return QString("%1 (%2)").arg(month.toString("MMMM")).arg(
							QString::number(ChildrenCount()));
				}
				break;
			case Day:
				return QString("%1").arg(QString::number(value), 2, '0') +
						QString(" (%1)").arg(QString::number(ChildrenCount()));
				break;
			case MonthAndDay:
				Q_ASSERT(false);
				break;
			default:
				Q_ASSERT(false);
				break;
		}
	} else {
		return QVariant();
	}
	return QVariant();
}

// virtual
bool DateModelItem::LessThan(const BaseModelItem* item) const {
	if (item->DataType() != BaseModelItem::date) {
		return BaseModelItem::LessThan(item);
	}
	const DateModelItem* dateItem = dynamic_cast<const DateModelItem*>(item);
	return value < dateItem->value;
}
