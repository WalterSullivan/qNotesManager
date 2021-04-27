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

#ifndef SEARCHRESULTITEMDELEGATE_H
#define SEARCHRESULTITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QStringList>

namespace qNotesManager {
	class SearchResultItemDelegate : public QStyledItemDelegate {
	Q_OBJECT
	public:
		explicit SearchResultItemDelegate(QObject *parent = nullptr);

		/*virtual*/ void paint (QPainter* painter, const QStyleOptionViewItem& option,
							const QModelIndex& index) const;
	private:
		QStringList splitStrings(const QString& text, const int start, const int length) const;
		QList<QRect> calculateRects(const QStringList& list, const QPoint& offset, int height,
									const QFontMetrics& metrics) const;
	};
}

#endif // SEARCHRESULTITEMDELEGATE_H
