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

#include "custommessagebox.h"

#include <QPushButton>
#include <QStyle>
#include <QHBoxLayout>

#include "appinfo.h"

using namespace qNotesManager;

CustomMessageBox::CustomMessageBox(const QString& text,
					   const QString& caption,
					   QMessageBox::Icon Icon,
					   QMessageBox::StandardButtons buttons,
					   QMessageBox::StandardButton defaultButton,
					   QWidget* parent) :
	QDialog(parent, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint) {
	clickedButton = 0;

	if (!caption.isEmpty()) {
		captionLabel = new QLabel(caption);
		QFont font = captionLabel->font();
		font.setBold(true);
		captionLabel->setFont(font);
	} else {
		captionLabel = 0;
	}

	if (Icon == QMessageBox::NoIcon) {
		imageLabel = 0;
	} else {
		imageLabel = new QLabel();
		imageLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		imageLabel->setPixmap(standardIcon(Icon));

	}

	textLabel = new QLabel(text);

	buttonsBox = new QDialogButtonBox();
	QObject::connect(buttonsBox, SIGNAL(clicked(QAbstractButton*)),
					 this, SLOT(sl_ButtonBox_ButtonClicked(QAbstractButton*)));

	uint mask = QMessageBox::FirstButton;
	bool defaultButtonSet = false;
	while (mask <= QMessageBox::LastButton) {

		uint sb = buttons & mask;
		mask <<= 1;
		if (!sb) {
			continue;
		}
		QPushButton* button = buttonsBox->addButton((QDialogButtonBox::StandardButton)sb);
		// Choose the first accept role as the default
		if (defaultButtonSet) {
			continue;
		}
		if ((defaultButton == QMessageBox::NoButton && buttonsBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
			|| (defaultButton != QMessageBox::NoButton && sb == uint(defaultButton))) {
			button->setDefault(true);
			button->setFocus();
			defaultButtonSet = true;
		}
	}

	layout = new QVBoxLayout();

	if (captionLabel) {
		layout->addWidget(captionLabel);
	}

	QHBoxLayout* hl = new QHBoxLayout();
	if (imageLabel) {
		hl->addWidget(imageLabel);
	}
	hl->addWidget(textLabel);
	hl->setContentsMargins(0, 0, 0, 0);
	layout->addLayout(hl);


	layout->addWidget(buttonsBox);
	layout->setSpacing(10);

	setLayout(layout);

	setWindowTitle(VER_FILEDESCRIPTION_STR);
}

QMessageBox::StandardButton CustomMessageBox::show() {
	if (this->exec() == -1) {
		return QMessageBox::Cancel;
	}

	if (clickedButton == 0) {
		return QMessageBox::Cancel;
	}

	return static_cast<QMessageBox::StandardButton>(buttonsBox->standardButton(clickedButton));
}

void CustomMessageBox::AddCustomWidget(QWidget* w) {
	if (!w) {return;}

	int buttonsIndex = layout->indexOf(buttonsBox);
	layout->insertWidget(buttonsIndex, w);
	if (w->parent() == 0) {w->setParent(this);}
}

QPixmap CustomMessageBox::standardIcon(QMessageBox::Icon icon) {
	QStyle *style = this->style();
	int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, this);

	QIcon tmpIcon;
	switch (icon) {
		case QMessageBox::Information:
		tmpIcon = style->standardIcon(QStyle::SP_MessageBoxInformation, 0, this);
		break;
		case QMessageBox::Warning:
		tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, 0, this);
		break;
		case QMessageBox::Critical:
		tmpIcon = style->standardIcon(QStyle::SP_MessageBoxCritical, 0, this);
		break;
		case QMessageBox::Question:
		tmpIcon = style->standardIcon(QStyle::SP_MessageBoxQuestion, 0, this);
		default:
		break;
	}

	if (!tmpIcon.isNull()) {
		return tmpIcon.pixmap(iconSize, iconSize);
	}
	return QPixmap();
}

void CustomMessageBox::sl_ButtonBox_ButtonClicked(QAbstractButton* button) {
	clickedButton = button;

	accept();
}
