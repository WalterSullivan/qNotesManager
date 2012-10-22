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

#include "separatoritemdelegate.h"
#include "basemodelitem.h"

#include <QPainter>
#include <QDebug>
#include <QItemEditorFactory>

using namespace qNotesManager;

SeparatorItemDelegate::SeparatorItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {

	setItemEditorFactory(new QItemEditorFactory());
}

/*virtual*/
void SeparatorItemDelegate::paint (QPainter* painter, const QStyleOptionViewItem& option,
						const QModelIndex& index) const {
	BaseModelItem* item = static_cast<BaseModelItem*>(index.internalPointer());
	Q_ASSERT(item != 0);
	if (item->DataType() != BaseModelItem::Separator) {
		QStyledItemDelegate::paint(painter, option, index);
		return;
	}

	const int lineWidth = 2;
	const int horizontalOffset = 5;
	const int verticalOffset = option.rect.height() / 2 + option.rect.height() % 2;

	QLinearGradient gradient(option.rect.x(), 0, option.rect.width(), 0);
	gradient.setColorAt(0, Qt::white);
	gradient.setColorAt(0.2, Qt::black);
	gradient.setColorAt(0.8, Qt::black);
	gradient.setColorAt(1, Qt::white);
	QBrush lineBrush(gradient);
	QPen linePen(lineBrush, lineWidth);

	const QLine line(option.rect.x() + horizontalOffset,
			   option.rect.y() + verticalOffset,
			   option.rect.x() + option.rect.width() - horizontalOffset,
			   option.rect.y() + verticalOffset);

	painter->save();
		painter->setPen(linePen);
		painter->drawLine(line);
	painter->restore();
}
