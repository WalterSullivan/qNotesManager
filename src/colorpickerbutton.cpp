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

#include "colorpickerbutton.h"

#include "global.h"

#include <QColorDialog>
#include <QPainter>
#include <QPixmap>

using namespace qNotesManager;

ColorPickerButton::ColorPickerButton(AffectedObject type, QWidget *parent) :
		QToolButton(parent),
		colorType(type),
		maxColorsListSize(5)
{
	switch (type) {
		case TextBackgroundColor:
			defaultColor = QColor(0, 0, 0, 0);
			break;
		case TextForegroundColor:
			defaultColor = QColor(0, 0, 0, 255);
			break;
		case TextUnderlineColor:
			defaultColor = QColor(0, 0, 0, 0);
			break;
		default:
			WARNING("Unhandled case branch");
			break;
	};

	avaliableColors.append(ColorEntry(Qt::red, "Red"));
	avaliableColors.append(ColorEntry(Qt::green, "Green"));
	avaliableColors.append(ColorEntry(Qt::blue, "Blue"));

	pickCustomColorAction = new QAction(tr("Custom color..."), this);
	defaultColorAction = new QAction("Default color", this);

	menu = new QMenu(this);
	QObject::connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(sl_ActionTriggered(QAction*)));

	FillColorsList();


	this->setMenu(menu);
	this->setFocusPolicy(Qt::NoFocus);
	this->setPopupMode(QToolButton::MenuButtonPopup);

	SetCurrentColor(defaultColor);
}

ColorPickerButton::~ColorPickerButton(){}

void ColorPickerButton::FillColorsList() {
	SortColorsList();

	QBrush brush;
	brush.setStyle(Qt::SolidPattern);
	QPainter painter;

	menu->clear(); // menu deletes all 'color' actions
	menu->addAction(defaultColorAction);
	menu->addSeparator();

	QColor color;

	for (int i = 0; i < maxColorsListSize; ++i) {
		if (i >= avaliableColors.size()) {
			break;
		}
		ColorEntry entry = avaliableColors[i];
		color = entry.color;
		brush.setColor(color);
		QPixmap pixmap(iconSize());
		painter.begin(&pixmap);
			painter.setPen(Qt::NoPen);
			painter.setBrush(brush);
			painter.drawRect(QRect(QPoint(0, 0), iconSize()));
			painter.end();
		QIcon icon(pixmap);

		QAction* action = new QAction(icon, entry.name, menu);
		action->setData(color);

		menu->addAction(action);
	}

	menu->addSeparator();
	menu->addAction(pickCustomColorAction);
}

QColor ColorPickerButton::GetCurrentColor() const {
	return currentColor;
}

void ColorPickerButton::SetCurrentColor(QColor& color) {
	if (currentColor == color) {return;}
	currentColor = color;
}

int ColorPickerButton::GetListSize() const {
	return maxColorsListSize;
}

void ColorPickerButton::SetListSize(int size) {
	if (size > 0 && size < 30) {
		maxColorsListSize = size;
	}
}

void ColorPickerButton::SortColorsList() {
	int border = 0;
	while (border < avaliableColors.size() - 1) {
		for (int i = border; i < avaliableColors.size() - 1; ++i) {
			ColorEntry entry1 = avaliableColors[i];
			ColorEntry entry2 = avaliableColors[i + 1];

			if (entry2.weight > entry1.weight) {
				avaliableColors[i] = entry2;
				avaliableColors[i + 1] = entry1;
			}
		}
		border++;
	}
}

void ColorPickerButton::sl_ActionTriggered(QAction* action) {
	QColor c;
	if (action == defaultColorAction) {
		c = defaultColor;
	} else {
		if (action == pickCustomColorAction) {
			QColor newColor = QColorDialog::getColor(currentColor, this);
			if (!newColor.isValid()) {
				return;
			}
			c = newColor;
		} else {
			c = action->data().value<QColor>();
		}

		bool newColorInList = false;
		for (int i = 0; i < avaliableColors.size(); ++i) {
			ColorEntry& ce = avaliableColors[i];
			if (ce.color == c) {
				ce.weight++;
				newColorInList = true;
				break;
			}
		}
		if (!newColorInList) {
			ColorEntry entry(c, "Custom");
			avaliableColors.append(entry);
		}

		FillColorsList();
	}

	SetCurrentColor(c);

	emit sg_ColorSelected(c);
	emit clicked();
}

