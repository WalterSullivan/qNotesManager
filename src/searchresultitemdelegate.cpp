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

#include "searchresultitemdelegate.h"

#include "searchmodelitem.h"
#include "global.h"

#include <QPainter>
#include <QApplication>
#include <QDebug>

using namespace qNotesManager;

SearchResultItemDelegate::SearchResultItemDelegate(QObject *parent) : QStyledItemDelegate (parent) {
}

/* virtual */
void SearchResultItemDelegate::paint (QPainter* painter, const QStyleOptionViewItem& option,
									  const QModelIndex& index) const {
	BaseModelItem* item = static_cast<BaseModelItem*>(index.internalPointer());
	if (item == nullptr) {
		WARNING("Null pointer recieved");
		return;
	}
	if (item->DataType() != BaseModelItem::SearchResult) {
		QStyledItemDelegate::paint(painter, option, index);
		return;
	}

	bool drawIcon = false;
	QPixmap icon;
	if (index.model()->data(index, Qt::DecorationRole).canConvert<QPixmap>()) {
		icon = index.model()->data(index, Qt::DecorationRole).value<QPixmap>();
		drawIcon = true;
	}

	const int iconTextInterval = 6;
	const int iconOffset = 1;

	const QString text = index.model()->data(index, Qt::DisplayRole).toString();
	QVariant tempVariant = index.model()->data(index, SearchModelItem::HighlightStartRole);
	if (tempVariant.isNull()) {
		WARNING("Could not retrieve data from item");
		tempVariant = QVariant(0);
	}
	const int matchStart = tempVariant.toInt();

	tempVariant = index.model()->data(index, SearchModelItem::HightlightLengthRole);
	if (tempVariant.isNull()) {
		WARNING("Could not retrieve data from item");
		tempVariant = QVariant(0);
	}
	const int matchLength = tempVariant.toInt();

	QStyleOptionViewItem opt(option);
	initStyleOption(&opt, index);
	painter->save();

	QStyle *style = QApplication::style();
	style->drawControl(QStyle::CE_ItemViewItem, &option, painter, 0);

	QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled;
	if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
		cg = QPalette::Inactive;
	}

	QPen normalPen;
	if (option.state & QStyle::State_Selected) {
		normalPen = QPen(option.palette.color(cg, QPalette::HighlightedText));
	} else {
		normalPen = QPen(option.palette.color(cg, QPalette::Text));
	}

	const QBrush highlightBrush(QColor(255, 255, 0), Qt::SolidPattern);
	const QBrush selectedHighlightBrush(QColor(130, 130, 0), Qt::SolidPattern);

	const QStringList strings = splitStrings(text, matchStart, matchLength);
	const QPoint textOffset = drawIcon ? QPoint(option.rect.topLeft().x() + iconOffset + icon.width()
										  + iconTextInterval, option.rect.topLeft().y())
										: option.rect.topLeft();
	const QList<QRect> rects = calculateRects(strings, textOffset,
											  option.rect.height(), painter->fontMetrics());

	painter->setPen(normalPen);

	const QRect iconRect(option.rect.topLeft().x() + iconOffset,
						 option.rect.topLeft().y() + iconOffset,
						 icon.width(),
						 icon.height());
	if (drawIcon) {
		painter->drawPixmap(iconRect, icon);
	}

	painter->drawText(rects.value(0), Qt::AlignLeft | Qt::AlignVCenter, strings.value(0));
	painter->save();
	painter->setBackgroundMode(Qt::OpaqueMode);
	if (!(option.state & QStyle::State_Selected)) {
		painter->setBackground(highlightBrush);
	} else {
		painter->setBackground(selectedHighlightBrush);
	}
	painter->drawText(rects.value(1), Qt::AlignLeft | Qt::AlignVCenter, strings.value(1));
	painter->restore();
	painter->drawText(rects.value(2), Qt::AlignLeft | Qt::AlignVCenter, strings.value(2));

	painter->restore();
}

QStringList SearchResultItemDelegate::splitStrings(const QString& text, const int start,
												   const int length) const {
	QStringList list;
	list << text.mid(0, start);
	list << text.mid(start, length);
	list << text.mid(start + length, text.length() - start - length);
	return list;
}

QList<QRect> SearchResultItemDelegate::calculateRects(const QStringList& list,
													  const QPoint& offset,
													  int height,
													  const QFontMetrics& metrics) const {
	int actualHeight = qMax(height, metrics.height());
	const QRect firstStringRect = QRect(0, 0, metrics.width(list.value(0)), actualHeight);
	const QRect secondStringRect = QRect(0, 0, metrics.width(list.value(1)), actualHeight);
	const QRect thirdStringRect = QRect(0, 0, metrics.width(list.value(2)), actualHeight);

	const QRect firstStringFinalRect(offset, firstStringRect.size());

	const QRect secondStringFinalRect(offset.x() + firstStringRect.width(), offset.y(),
								 secondStringRect.width(), secondStringRect.height());

	const QRect thirdStringFinalRect(offset.x() + firstStringRect.width() + secondStringRect.width(),
								offset.y(), thirdStringRect.width(), thirdStringRect.height());
	QList<QRect> rects;
	rects << firstStringFinalRect << secondStringFinalRect << thirdStringFinalRect;
	return rects;
}
