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

#include "sizeeditwidget.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

using namespace qNotesManager;

SizeEditWidget::SizeEditWidget(QWidget *parent) : QDialog(parent), NewSize(QSize()) {
	// Buttons
	buttonBox = new QDialogButtonBox(this);
	QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(sl_ButtonBox_Clicked(QAbstractButton*)));
	buttonBox->addButton(QDialogButtonBox::Ok)->setDefault(true);
	buttonBox->addButton(QDialogButtonBox::Cancel)->setAutoDefault(false);
	buttonBox->addButton(QDialogButtonBox::Reset)->setAutoDefault(false);


	widthLabel = new QLabel("Width:", this);

	widthSlider = new QSlider(Qt::Horizontal, this);
	widthSlider->setMinimum(minimalImageSize);
	widthSlider->setMaximum(maximalImageSize);
	widthSlider->setTickPosition(QSlider::TicksBelow);
	QObject::connect(widthSlider, SIGNAL(valueChanged(int)),
					 this, SLOT(sl_WidthSlider_ValueChanged(int)));

	widthSpinBox = new QSpinBox(this);
	widthSpinBox->setMinimum(minimalImageSize);
	widthSpinBox->setMaximum(maximalImageSize);
	widthSpinBox->setSuffix("px");
	QObject::connect(widthSpinBox, SIGNAL(valueChanged(int)),
					 this, SLOT(sl_WidthSpinBox_ValueChanged(int)));


	heightLabel = new QLabel("Height:", this);

	heightSlider = new QSlider(Qt::Horizontal, this);
	heightSlider->setMinimum(minimalImageSize);
	heightSlider->setMaximum(maximalImageSize);
	heightSlider->setTickPosition(QSlider::TicksBelow);
	QObject::connect(heightSlider, SIGNAL(valueChanged(int)),
					  this, SLOT(sl_HeightSlider_ValueChanged(int)));

	heightSpinBox = new QSpinBox(this);
	heightSpinBox->setMinimum(minimalImageSize);
	heightSpinBox->setMaximum(maximalImageSize);
	heightSpinBox->setSuffix("px");
	QObject::connect(heightSpinBox, SIGNAL(valueChanged(int)),
					 this, SLOT(sl_HeightSpinBox_ValueChanged(int)));


	keepRationButton = new QToolButton(this);
	keepRationButton->setCheckable(true);
	keepRationButton->setToolTip("Keep aspect ratio");
	keepRationButton->setIcon(QIcon(":/gui/chain"));
	QObject::connect(keepRationButton, SIGNAL(toggled(bool)),
					 this, SLOT(sl_KeepRatioButton_Toggled()));

	QGridLayout* gridLayout = new QGridLayout();
	gridLayout->addWidget(widthLabel, 0, 0);
	gridLayout->addWidget(widthSlider, 0, 1);
	gridLayout->addWidget(widthSpinBox, 0, 2);

	gridLayout->addWidget(heightLabel, 1, 0);
	gridLayout->addWidget(heightSlider, 1, 1);
	gridLayout->addWidget(heightSpinBox, 1, 2);

	gridLayout->addWidget(keepRationButton, 0, 3, 2, 1);

	gridLayout->setSpacing(10);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(gridLayout);
	mainLayout->addStretch();
	mainLayout->addWidget(buttonBox);

	setLayout(mainLayout);
	setWindowTitle("Resize image");
	setWindowIcon(QIcon(":/gui/image-resize"));
	resize(400, height());
}

void SizeEditWidget::SetData(const QSize& currentSize, const QSize& originalSize) {
	imageCurrentSize = currentSize;
	imageOriginalSize = originalSize;

	if (!originalSize.isEmpty()) {
		imageOriginalRatio = (double)originalSize.width() / (double)originalSize.height();

		widthSlider->setMaximum(originalSize.width());
		heightSlider->setMaximum(originalSize.height());

		widthSpinBox->setMaximum(originalSize.width());
		heightSpinBox->setMaximum(originalSize.height());

	} else {
		imageOriginalRatio = 1.0;

		widthSlider->setMaximum(maximalImageSize);
		heightSlider->setMaximum(maximalImageSize);

		widthSpinBox->setMaximum(maximalImageSize);
		heightSpinBox->setMaximum(maximalImageSize);
	}
	widthSlider->setTickInterval(widthSlider->maximum() / 10);
	heightSlider->setTickInterval(heightSlider->maximum() / 10);

	if (!currentSize.isEmpty()) {
		widthSlider->setValue(currentSize.width());
		heightSlider->setValue(currentSize.height());

		widthSpinBox->setValue(currentSize.width());
		heightSpinBox->setValue(currentSize.height());
	} else {
		widthSlider->setValue(originalSize.width());
		heightSlider->setValue(originalSize.height());

		widthSpinBox->setValue(originalSize.width());
		heightSpinBox->setValue(originalSize.height());
	}
}

void SizeEditWidget::sl_ButtonBox_Clicked(QAbstractButton* button) {
	QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(button);

	switch(role) {
		case QDialogButtonBox::AcceptRole:
			accept();
			break;
		case QDialogButtonBox::RejectRole:
			reject();
			break;
		case QDialogButtonBox::ResetRole:
			SetData(imageOriginalSize, imageOriginalSize);
			break;
		default:
			reject();
	}
}

void SizeEditWidget::accept() {
	NewSize = QSize(widthSlider->value(), heightSlider->value());
	QDialog::accept();
}

void SizeEditWidget::sl_WidthSlider_ValueChanged(int value) {
	widthSpinBox->blockSignals(true);
	widthSpinBox->setValue(value);
	widthSpinBox->blockSignals(false);

	if (keepRationButton->isChecked()) {
		int newHeight = (int)((double)value / imageOriginalRatio);

		heightSlider->blockSignals(true);
		heightSlider->setValue(newHeight);
		heightSlider->blockSignals(false);

		heightSpinBox->blockSignals(true);
		heightSpinBox->setValue(newHeight);
		heightSpinBox->blockSignals(false);
	}
}

void SizeEditWidget::sl_HeightSlider_ValueChanged(int value) {
	heightSpinBox->blockSignals(true);
	heightSpinBox->setValue(value);
	heightSpinBox->blockSignals(false);

	if (keepRationButton->isChecked()) {
		int newWidth = (int)((double)value * imageOriginalRatio);

		widthSlider->blockSignals(true);
		widthSlider->setValue(newWidth);
		widthSlider->blockSignals(false);

		widthSpinBox->blockSignals(true);
		widthSpinBox->setValue(newWidth);
		widthSpinBox->blockSignals(false);
	}
}

void SizeEditWidget::sl_WidthSpinBox_ValueChanged(int value) {
	widthSlider->blockSignals(true);
	widthSlider->setValue(value);
	widthSlider->blockSignals(false);

	if (keepRationButton->isChecked()) {
		int newHeight = (int)((double)value / imageOriginalRatio);

		heightSlider->blockSignals(true);
		heightSlider->setValue(newHeight);
		heightSlider->blockSignals(false);

		heightSpinBox->blockSignals(true);
		heightSpinBox->setValue(newHeight);
		heightSpinBox->blockSignals(false);
	}
}

void SizeEditWidget::sl_HeightSpinBox_ValueChanged(int value) {
	heightSlider->blockSignals(true);
	heightSlider->setValue(value);
	heightSlider->blockSignals(false);

	if (keepRationButton->isChecked()) {
		int newWidth = (int)((double)value * imageOriginalRatio);

		widthSlider->blockSignals(true);
		widthSlider->setValue(newWidth);
		widthSlider->blockSignals(false);

		widthSpinBox->blockSignals(true);
		widthSpinBox->setValue(newWidth);
		widthSpinBox->blockSignals(false);
	}
}

void SizeEditWidget::sl_KeepRatioButton_Toggled() {
	if (keepRationButton->isChecked()) {
		int newHeight = (int)((double)widthSpinBox->value() / imageOriginalRatio);

		heightSlider->blockSignals(true);
		heightSlider->setValue(newHeight);
		heightSlider->blockSignals(false);

		heightSpinBox->blockSignals(true);
		heightSpinBox->setValue(newHeight);
		heightSpinBox->blockSignals(false);
	}
}
