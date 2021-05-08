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

#include "paragraphpropertieswidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QDebug>
#include <QMap>
#include <QToolTip>

using namespace qNotesManager;

ParagraphPropertiesWidget::ParagraphPropertiesWidget(QWidget *parent) : QDialog(parent) {
	alignmentGroupBox = new QGroupBox("Alignment");
	leftAlignmentRadioButton = new QRadioButton("Left");
	centerAlignmentRadioButton = new QRadioButton("Center");
	rightAlignmentRadioButton = new QRadioButton("Right");
	justifyAlignmentRadioButton = new QRadioButton("Align justify");

	textIndentLabel = new QLabel("First line indent");
	textIndentSpinBox = new QSpinBox();
	textIndentSpinBox->setSuffix("px");
	textIndentSpinBox->setMinimum(-1000);
	textIndentSpinBox->setMaximum(1000);

	nonBreakableLinesCheckbox = new QCheckBox("Non-breakable lines");

	QGroupBox* marginsGroupBox = new QGroupBox("Margins");
	leftMarginLabel = new QLabel("Left margin");
	leftMarginSpinBox = new QSpinBox();
	leftMarginSpinBox->setSuffix("px");
	leftMarginSpinBox->setMinimum(0);
	leftMarginSpinBox->setMaximum(1000);
	rightMarginLabel = new QLabel("Right margin");
	rightMarginSpinBox = new QSpinBox();
	rightMarginSpinBox->setSuffix("px");
	rightMarginSpinBox->setMinimum(0);
	rightMarginSpinBox->setMaximum(1000);
	topMarginLabel = new QLabel("Top margin");
	topMarginSpinBox = new QSpinBox();
	topMarginSpinBox->setSuffix("px");
	topMarginSpinBox->setMinimum(0);
	topMarginSpinBox->setMaximum(1000);
	bottomMarginLabel = new QLabel("Bottom margin");
	bottomMarginSpinBox = new QSpinBox();
	bottomMarginSpinBox->setSuffix("px");
	bottomMarginSpinBox->setMinimum(0);
	bottomMarginSpinBox->setMaximum(1000);

#if QT_VERSION >= 0x040800
	lineHeightGroupBox = new QGroupBox("Spacing", this);
	lineHeightLabel = new QLabel("of");
	lineHeightSpinBox = new QSpinBox();
	lineHeightSpinBox->setMinimum(-1000);
	lineHeightSpinBox->setMaximum(1000);
	lineHeightTypeLabel = new QLabel("Line height type");
	lineHeightTypeComboBox = new QComboBox();
	QObject::connect(lineHeightTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_LineHeightTypeComboBox_CurrentIndexChanged(int)));
	lineHeightTypeComboBox ->addItem("Single spacing", QTextBlockFormat::SingleHeight);
	lineHeightTypeComboBox ->addItem("Proportional spacing", QTextBlockFormat::ProportionalHeight);
	lineHeightTypeComboBox ->addItem("Fixed line height", QTextBlockFormat::FixedHeight);
	lineHeightTypeComboBox ->addItem("Minimum line height", QTextBlockFormat::MinimumHeight);
	lineHeightTypeComboBox ->addItem("Spacing between lines", QTextBlockFormat::LineDistanceHeight);
#endif

	// Buttons
	buttonBox = new QDialogButtonBox(this);
	QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(sl_ButtonBox_Clicked(QAbstractButton*)));
	buttonBox->addButton(QDialogButtonBox::Ok)->setDefault(true);
	buttonBox->addButton(QDialogButtonBox::Apply)->setAutoDefault(false);
	buttonBox->addButton(QDialogButtonBox::Cancel)->setAutoDefault(false);

	QHBoxLayout* alignmentLayout = new QHBoxLayout();
	alignmentLayout->addWidget(leftAlignmentRadioButton);
	alignmentLayout->addWidget(centerAlignmentRadioButton);
	alignmentLayout->addWidget(rightAlignmentRadioButton);
	alignmentLayout->addWidget(justifyAlignmentRadioButton);
	alignmentGroupBox->setLayout(alignmentLayout);

	QGridLayout* marginsLayout = new QGridLayout();
	marginsLayout->addWidget(topMarginLabel, 0, 1);
	marginsLayout->addWidget(topMarginSpinBox, 1, 1);
	marginsLayout->addWidget(leftMarginLabel, 2, 0);
	marginsLayout->addWidget(leftMarginSpinBox, 3, 0);
	marginsLayout->addWidget(rightMarginLabel, 2, 2);
	marginsLayout->addWidget(rightMarginSpinBox, 3, 2);
	marginsLayout->addWidget(bottomMarginLabel, 4, 1);
	marginsLayout->addWidget(bottomMarginSpinBox, 5, 1);
	marginsGroupBox->setLayout(marginsLayout);

	QGridLayout* layout = new QGridLayout();
	layout->addWidget(alignmentGroupBox, 0, 0, 1, 2);
	layout->addWidget(textIndentLabel, 2, 0);
	layout->addWidget(textIndentSpinBox, 2, 1);
	layout->addWidget(nonBreakableLinesCheckbox, 3, 0, 1, 2);
	layout->addWidget(marginsGroupBox, 4, 0, 1, 2);

#if QT_VERSION >= 0x040800
	QHBoxLayout* lineHeightLayout = new QHBoxLayout();
	lineHeightLayout->addWidget(lineHeightTypeLabel);
	lineHeightLayout->addWidget(lineHeightTypeComboBox);
	lineHeightLayout->addWidget(lineHeightLabel, 0, Qt::AlignHCenter);
	lineHeightLayout->addWidget(lineHeightSpinBox);
	lineHeightGroupBox->setLayout(lineHeightLayout);
	layout->addWidget(lineHeightGroupBox, 8, 0, 1, 2);
#endif

	// Main layout
	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(layout);
	mainLayout->addStretch();
	mainLayout->addWidget(buttonBox);
	mainLayout->setContentsMargins(5, 5, 5, 5);

	setLayout(mainLayout);
	setWindowTitle("Paragraph properties");
	setWindowIcon(QIcon(":/gui/paragraph"));
	resize(400, 100);
}

void ParagraphPropertiesWidget::SetCursor(QTextCursor c) {
	cursor = c;

	loadData();
}

void ParagraphPropertiesWidget::loadData() {
	if (cursor.isNull()) {
		qWarning() << "Cursor is invalid";
		return;
	}

	QTextBlockFormat format = cursor.blockFormat();
	if (!format.isValid()) {
		qWarning() << "Format is invalid";
		return;
	}

	if ((format.alignment() & Qt::AlignLeft) == Qt::AlignLeft) {
		leftAlignmentRadioButton->setChecked(true);
	} else if ((format.alignment() & Qt::AlignHCenter) == Qt::AlignHCenter) {
		centerAlignmentRadioButton->setChecked(true);
	} else if ((format.alignment() & Qt::AlignRight) == Qt::AlignRight) {
		rightAlignmentRadioButton->setChecked(true);
	} else if ((format.alignment() & Qt::AlignJustify) == Qt::AlignJustify) {
		justifyAlignmentRadioButton->setChecked(true);
	}

	textIndentSpinBox->setValue(format.textIndent());
	nonBreakableLinesCheckbox->setChecked(format.nonBreakableLines());
	leftMarginSpinBox->setValue(format.leftMargin());
	rightMarginSpinBox->setValue(format.rightMargin());
	topMarginSpinBox->setValue(format.topMargin());
	bottomMarginSpinBox->setValue(format.bottomMargin());

#if QT_VERSION >= 0x040800
	int index = lineHeightTypeComboBox->findData(format.lineHeightType());
	lineHeightTypeComboBox->setCurrentIndex(index);
	lineHeightSpinBox->setValue(format.lineHeight());
#endif
}

bool ParagraphPropertiesWidget::applyFormat() {
	if (lineHeightTypeComboBox->currentIndex() == -1) {
		QToolTip::showText(lineHeightGroupBox->mapToGlobal(lineHeightTypeComboBox->pos()), "Select line height type", lineHeightTypeComboBox);
		return false;
	}

	QTextBlockFormat format;

	if (leftAlignmentRadioButton->isChecked()) {
		format.setAlignment(Qt::AlignLeft);
	} else if (centerAlignmentRadioButton->isChecked()) {
		format.setAlignment(Qt::AlignCenter);
	} else if (rightAlignmentRadioButton->isChecked()) {
		format.setAlignment(Qt::AlignRight);
	} else if (justifyAlignmentRadioButton->isChecked()) {
		format.setAlignment(Qt::AlignJustify);
	}

	format.setTextIndent(textIndentSpinBox->value());
	format.setNonBreakableLines(nonBreakableLinesCheckbox->isChecked());
	format.setLeftMargin(leftMarginSpinBox->value());
	format.setRightMargin(rightMarginSpinBox->value());
	format.setTopMargin(topMarginSpinBox->value());
	format.setBottomMargin(bottomMarginSpinBox->value());

#if QT_VERSION >= 0x040800
	format.setLineHeight(
				lineHeightSpinBox->value(),
				(QTextBlockFormat::LineHeightTypes)lineHeightTypeComboBox->itemData(lineHeightTypeComboBox->currentIndex()).toInt());
#endif

	cursor.setBlockFormat(format);
	return true;
}

#if QT_VERSION >= 0x040800
void ParagraphPropertiesWidget::sl_LineHeightTypeComboBox_CurrentIndexChanged(int index) {
	QTextBlockFormat::LineHeightTypes heightType = (QTextBlockFormat::LineHeightTypes)lineHeightTypeComboBox->itemData(index).toInt();

	switch(heightType) {
		case QTextBlockFormat::SingleHeight:
			lineHeightLabel->setEnabled(false);
			lineHeightSpinBox->setEnabled(false);
			lineHeightSpinBox->setValue(0);
			lineHeightSpinBox->setSuffix("");
			break;
		case QTextBlockFormat::ProportionalHeight:
			lineHeightLabel->setEnabled(true);
			lineHeightSpinBox->setEnabled(true);
			lineHeightSpinBox->setValue(100);
			lineHeightSpinBox->setSuffix("%");
			break;
		case QTextBlockFormat::FixedHeight:
		case QTextBlockFormat::MinimumHeight:
			lineHeightLabel->setEnabled(true);
			lineHeightSpinBox->setEnabled(true);
			lineHeightSpinBox->setValue(20);
			lineHeightSpinBox->setSuffix("px");
			break;
		case QTextBlockFormat::LineDistanceHeight:
			lineHeightLabel->setEnabled(true);
			lineHeightSpinBox->setEnabled(true);
			lineHeightSpinBox->setValue(0);
			lineHeightSpinBox->setSuffix("px");
			break;
		default:
			lineHeightLabel->setEnabled(false);
			lineHeightSpinBox->setEnabled(false);
			lineHeightSpinBox->setValue(0);
			lineHeightSpinBox->setSuffix("");
			break;
	}
}
#endif

void ParagraphPropertiesWidget::sl_ButtonBox_Clicked(QAbstractButton* button) {
	QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(button);

	switch(role) {
		case QDialogButtonBox::AcceptRole:
			accept();
			break;
		case QDialogButtonBox::RejectRole:
			reject();
			break;
		case QDialogButtonBox::ApplyRole:
			applyFormat();
			break;
		default:
			reject();
	}
}

void ParagraphPropertiesWidget::accept() {
	if (applyFormat()) {
		QDialog::accept();
	}
}
