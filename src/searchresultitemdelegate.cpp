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

SearchResultItemDelegate::SearchResultItemDelegate(QObject *parent) : QItemDelegate (parent) {
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
		QItemDelegate::paint(painter, option, index);
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

	tempVariant = index.model()->data(index, SearchModelItem::ExpiredRole);
	if (tempVariant.isNull()) {
		WARNING("Could not retrieve data from item");
		tempVariant = QVariant(false);
	}
	const int expired = tempVariant.toBool();

	painter->save();

	QPalette::ColorGroup colorGroup = option.palette.currentColorGroup();
	if (expired) {
		colorGroup = QPalette::Disabled;
	}

	// Draw selected background
	if (option.state & QStyle::State_Selected) {
		painter->fillRect(option.rect, option.palette.brush(colorGroup, QPalette::Highlight));
	}

	// Draw icon
	const QRect iconRect(option.rect.topLeft().x() + iconOffset,
						 option.rect.topLeft().y() + iconOffset,
						 icon.width(),
						 icon.height());
	if (drawIcon) {
		painter->drawPixmap(iconRect, icon);
	}

	QFont highlightFont = option.font;
	highlightFont.setBold(true);

	// Draw text
	const QStringList strings = splitStrings(text, matchStart, matchLength);
	const QPoint textOffset = drawIcon ? QPoint(option.rect.topLeft().x() + iconOffset + icon.width()
										  + iconTextInterval, option.rect.topLeft().y())
										: option.rect.topLeft();
	const QList<QRect> rects = calculateRects(strings, textOffset,
											  option.rect.height(), painter->fontMetrics(),
											  highlightFont);

	QPen normalPen;
	if (option.state & QStyle::State_Selected) {
		normalPen = QPen(option.palette.color(colorGroup, QPalette::HighlightedText));
	} else {
		normalPen = QPen(option.palette.color(colorGroup, QPalette::Text));
	}

	painter->setPen(normalPen);
	painter->drawText(rects.value(0), Qt::AlignLeft | Qt::AlignVCenter, strings.value(0));

	painter->save();
	painter->setFont(highlightFont);
	if (!(option.state & QStyle::State_Selected)) {
		const QPen highlightPen = QPen(QColor(255, 0, 0));
		const QBrush highlightBrush(QColor(255, 255, 191), Qt::SolidPattern);
		painter->setBackgroundMode(Qt::OpaqueMode);
		painter->setBackground(highlightBrush);
		painter->setPen(highlightPen);
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
													  const QFontMetrics& metrics, const QFont& hightlightFont) const {
	int actualHeight = qMax(height, metrics.height());
	const QRect firstStringRect = QRect(0, 0, metrics.width(list.value(0)), actualHeight);
	const QRect secondStringRect = QRect(0, 0, QFontMetrics(hightlightFont).width(list.value(1)), actualHeight);
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
