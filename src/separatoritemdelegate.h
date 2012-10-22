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

#ifndef SEPARATORITEMDELEGATE_H
#define SEPARATORITEMDELEGATE_H

#include <QStyledItemDelegate>

namespace qNotesManager {
	class SeparatorItemDelegate : public QStyledItemDelegate {
	Q_OBJECT
	public:
		explicit SeparatorItemDelegate(QObject *parent = 0);
		/*virtual*/ void paint (QPainter* painter, const QStyleOptionViewItem& option,
							const QModelIndex& index) const;


	};
}

#endif // SEPARATORITEMDELEGATE_H
