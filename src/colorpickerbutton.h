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

#ifndef COLORPICKERBUTTON_H
#define COLORPICKERBUTTON_H

#include <QMenu>
#include <QToolButton>
#include <QList>
#include <QColor>

namespace qNotesManager {
	class ColorPickerButton : public QToolButton {
	Q_OBJECT
	public:
		enum AffectedObject {
			TextBackgroundColor,
			TextForegroundColor,
			TextUnderlineColor
		};

		explicit ColorPickerButton(AffectedObject type, QWidget *parent = 0);
		~ColorPickerButton();

		QColor GetCurrentColor() const;
		void SetCurrentColor(QColor& color);

		int GetListSize() const;
		void SetListSize(int size);

	private:
		struct ColorEntry {
				ColorEntry(QColor c, int w) : color(c), weight(w) {}
				ColorEntry(QColor c) : color(c), weight(1) {}
				QColor color;
				int weight;
		};

		const AffectedObject	colorType;
		QColor					defaultColor;
		QColor					currentColor;
		int						maxColorsListSize;
		QMenu					*menu;
		QList<ColorEntry>		avaliableColors;
		QAction*				pickCustomColorAction;
		QAction*				defaultColorAction;
		QImage					currentImage;


		void SortColorsList();
		void FillColorsList();

	signals:
		void sg_ColorSelected(QColor);

	private slots:
		void sl_ActionTriggered(QAction*);

	};
}

#endif // COLORPICKERBUTTON_H
