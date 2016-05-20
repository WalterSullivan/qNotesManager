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

#include "tablepropertieswidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QColorDialog>
#include <QPainter>

using namespace qNotesManager;

TablePropertiesWidget::TablePropertiesWidget(QTextTable* t, QWidget* parent) : QDialog(parent) {
	this->table = t;

	tabWidget = new QTabWidget();

	// Table tab
	alignmentGroupBox = new QGroupBox("Alignment");
	leftAlignmentRadioButton = new QRadioButton("Left");
	centerAlignmentRadioButton = new QRadioButton("Center");
	rightAlignmentRadioButton = new QRadioButton("Right");
	QHBoxLayout* alignmentLayout = new QHBoxLayout();
	alignmentLayout->addWidget(leftAlignmentRadioButton);
	alignmentLayout->addWidget(centerAlignmentRadioButton);
	alignmentLayout->addWidget(rightAlignmentRadioButton);
	alignmentGroupBox->setLayout(alignmentLayout);

	cellPaddingLabel = new QLabel("Cell padding");
	cellPaddingSpinBox = new QDoubleSpinBox();
	cellPaddingSpinBox->setSuffix("px");

	cellSpacingLabel = new QLabel("Cell spacing");
	cellSpacingSpinBox = new QDoubleSpinBox();
	cellSpacingSpinBox->setSuffix("px");

	// Border
	borderGroupBox = new QGroupBox("Border", this);
	borderWidthLabel = new QLabel("Width:", this);
	borderWidthSpinBox = new QDoubleSpinBox(this);
	borderWidthSpinBox->setSuffix("px");
	borderWidthSpinBox->setMinimum(0.5);
	borderWidthSpinBox->setMaximum(50);
	borderWidthSpinBox->setSingleStep(0.1);

	borderColorLabel = new QLabel("Color:", this);

	borderColorDisplayLabel = new QLabel(this);
	borderColorPixmap = QPixmap(30, 20);
	borderColorDisplayLabel->setPixmap(borderColorPixmap);

	selectBorderColorButton = new QPushButton("Change", this);
	QObject::connect(selectBorderColorButton, SIGNAL(pressed()),
					 this, SLOT(sl_SelectBorderColorButton_Pressed()));

	borderStyleLabel = new QLabel("Style:", this);
	borderStyleComboBox = new QComboBox(this);
	borderStyleComboBox->addItem("None", QTextFrameFormat::BorderStyle_None);
	borderStyleComboBox->addItem("Dotted", QTextFrameFormat::BorderStyle_Dotted);
	borderStyleComboBox->addItem("Dashed", QTextFrameFormat::BorderStyle_Dashed);
	borderStyleComboBox->addItem("Solid", QTextFrameFormat::BorderStyle_Solid);
	borderStyleComboBox->addItem("Double", QTextFrameFormat::BorderStyle_Double);
	borderStyleComboBox->addItem("DotDash", QTextFrameFormat::BorderStyle_DotDash);
	borderStyleComboBox->addItem("DotDotDash", QTextFrameFormat::BorderStyle_DotDotDash);
	borderStyleComboBox->addItem("Groove", QTextFrameFormat::BorderStyle_Groove);
	borderStyleComboBox->addItem("Ridge", QTextFrameFormat::BorderStyle_Ridge);
	borderStyleComboBox->addItem("Inset", QTextFrameFormat::BorderStyle_Inset);
	borderStyleComboBox->addItem("Outset", QTextFrameFormat::BorderStyle_Outset);

	QGridLayout* borderLayout = new QGridLayout();
	borderLayout->addWidget(borderWidthLabel, 0, 0);
	borderLayout->addWidget(borderWidthSpinBox, 0, 1, 1, 2);
	borderLayout->addWidget(borderColorLabel, 1, 0);
	borderLayout->addWidget(borderColorDisplayLabel, 1, 1);
	borderLayout->addWidget(selectBorderColorButton, 1, 2);
	borderLayout->addWidget(borderStyleLabel, 2, 0);
	borderLayout->addWidget(borderStyleComboBox, 2, 1, 1, 2);
	borderGroupBox->setLayout(borderLayout);

	// Padding
	paddingLabel = new QLabel("Padding:", this);
	paddingSpinBox = new QDoubleSpinBox(this);
	paddingSpinBox->setSuffix("px");
	paddingSpinBox->setMinimum(0);
	paddingSpinBox->setMaximum(500);

	QHBoxLayout* paddingLayout = new QHBoxLayout();
	paddingLayout->addWidget(paddingLabel);
	paddingLayout->addWidget(paddingSpinBox);

	// BG Color
	tableBGColorLabel = new QLabel("Background color:", this);

	tableBGColorDisplayLabel = new QLabel(this);
	tableBGColorPixmap = QPixmap(30, 20);
	tableBGColorDisplayLabel->setPixmap(tableBGColorPixmap);

	selectTableBGColorButton = new QPushButton("Change", this);
	QObject::connect(selectTableBGColorButton, SIGNAL(pressed()), this, SLOT(sl_SelectTableBGColorButton_Pressed()));

	resetTableBGColorButton = new QPushButton("Reset", this);
	QObject::connect(resetTableBGColorButton, SIGNAL(pressed()), this, SLOT(sl_ResetTableBGColorButton_Pressed()));

	QHBoxLayout* BGColorLayout = new QHBoxLayout();
	BGColorLayout->addWidget(tableBGColorLabel);
	BGColorLayout->addWidget(tableBGColorDisplayLabel);
	BGColorLayout->addWidget(selectTableBGColorButton);
	BGColorLayout->addWidget(resetTableBGColorButton);

	// 1st tab layout
	QGridLayout* tableTabLayout = new QGridLayout();
	tableTabLayout->addWidget(cellPaddingLabel, 0, 0);
	tableTabLayout->addWidget(cellPaddingSpinBox, 0, 1);
	tableTabLayout->addWidget(cellSpacingLabel, 1, 0);
	tableTabLayout->addWidget(cellSpacingSpinBox, 1, 1);
	tableTabLayout->addWidget(alignmentGroupBox, 2, 0, 1, 2);
	tableTabLayout->addWidget(borderGroupBox, 3, 0, 1, 2);
	tableTabLayout->addLayout(paddingLayout, 4, 0, 1, 2);
	tableTabLayout->addLayout(BGColorLayout, 5, 0, 1, 2);

	QWidget* tableTabWidget = new QWidget(tabWidget);
	tableTabWidget->setLayout(tableTabLayout);
	tabWidget->addTab(tableTabWidget, QIcon(), "Format");

	tableWidget = new QTableWidget();
	tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	tableWidget->setColumnCount(3);
	QObject::connect(tableWidget, SIGNAL(itemSelectionChanged()),
					 this, SLOT(sl_TableWidget_ItemSelectionChanged()));
	QStringList headers;
	headers << "Item" << "Constraint type" << "Value";
	tableWidget->setHorizontalHeaderLabels(headers);
	tableWidget->verticalHeader()->hide();
	tableWidget->horizontalHeader()->setStretchLastSection(true);

	groupBox = new QGroupBox("Size type");

	minimalSizeRadioButton = new QRadioButton("Minimal size");
	fixedSizeRadioButton = new QRadioButton("Fixed size");
	percentageSizeRadioButton = new QRadioButton("Percent");
	minimalSizeRadioButton->setChecked(true);
	QObject::connect(minimalSizeRadioButton, SIGNAL(toggled(bool)),
					 this, SLOT(sl_MinimalSizeRadioButton_Toggled()));
	QObject::connect(fixedSizeRadioButton, SIGNAL(toggled(bool)),
					 this, SLOT(sl_FixedSizeRadioButton_Toggled()));
	QObject::connect(percentageSizeRadioButton, SIGNAL(toggled(bool)),
					 this, SLOT(sl_PercentageSizeRadioButton_Toggled()));

	widthLineEdit = new QLineEdit();
	widthLineEdit->setEnabled(false);
	QObject::connect(widthLineEdit, SIGNAL(textEdited(QString)),
					 this, SLOT(sl_WidthLineEdit_TextEdited(QString)));

	widthSpinBox = new QSpinBox();
	widthSpinBox->setSuffix("%");
	widthSpinBox->setEnabled(false);
	widthSpinBox->setMinimum(0);
	widthSpinBox->setMaximum(100);
	QObject::connect(widthSpinBox, SIGNAL(valueChanged(int)),
					 this, SLOT(sl_WidthSpinBox_ValueChanged(int)));

	QGridLayout* sizeTypeLayout = new QGridLayout;
	sizeTypeLayout->addWidget(minimalSizeRadioButton, 0, 0, 1, 1);
	sizeTypeLayout->addWidget(fixedSizeRadioButton, 1, 0, 1, 1);
	sizeTypeLayout->addWidget(widthLineEdit, 1, 1, 1, 1);
	sizeTypeLayout->addWidget(percentageSizeRadioButton, 2, 0, 1, 1);
	sizeTypeLayout->addWidget(widthSpinBox, 2, 1, 1, 1);
	groupBox->setLayout(sizeTypeLayout);

	QVBoxLayout* sizeTypeLayout2 = new QVBoxLayout();
	sizeTypeLayout2->addWidget(groupBox);
	sizeTypeLayout2->addStretch();

	QHBoxLayout* sizeLayout = new QHBoxLayout();
	sizeLayout->addWidget(tableWidget);
	sizeLayout->addLayout(sizeTypeLayout2);

	QWidget* sizesWidget = new QWidget(tabWidget);
	sizesWidget->setLayout(sizeLayout);
	tabWidget->addTab(sizesWidget, QIcon(":gui/resize"), "Width");

	// Buttons
	okButton = new QPushButton("OK");
	QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	cancelButton = new QPushButton("Cancel");
	QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	applyButton = new QPushButton("Apply");
	QObject::connect(applyButton, SIGNAL(clicked()), this, SLOT(sl_ApplyButton_Clicked()));

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addWidget(applyButton);
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);

	// Main layout
	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(tabWidget);
	mainLayout->addLayout(buttonsLayout);
	mainLayout->setContentsMargins(5, 5, 5, 5);

	setLayout(mainLayout);
	setWindowTitle("Table properties");

	loadData();
}

void TablePropertiesWidget::loadData() {
	cellPaddingSpinBox->setValue(table->format().cellPadding());
	cellSpacingSpinBox->setValue(table->format().cellSpacing());

	switch (table->format().alignment()) {
	case Qt::AlignCenter:
		centerAlignmentRadioButton->setChecked(true);
		break;
	case Qt::AlignRight:
		rightAlignmentRadioButton->setChecked(true);
		break;
	case Qt::AlignLeft:
	default:
		leftAlignmentRadioButton->setChecked(true);
		break;
	}

	doNotReactOnDataChange = true;
	TableWidthConstraint = table->format().width();
	ColumnWidthConstraints = table->format().columnWidthConstraints();
	tableColumnsCount = table->columns();
	addMissingColumnWidthConstraints();
	tableWidget->setRowCount(table->columns() + 1);
	doNotReactOnDataChange = false;

	fillTableWidget();
	tableWidget->resizeColumnsToContents();
	tableWidget->setCurrentCell(0, 0);

	if (table->format().hasProperty(QTextFormat::FrameBorder)) {
		borderWidthSpinBox->setValue(table->format().border());
	} else {
		borderWidthSpinBox->setValue(0);
	}

	if (table->format().hasProperty(QTextFormat::FrameBorderBrush)) {
		currentBorderColor = table->format().borderBrush().color();
	} else {
		currentBorderColor = QColor();
	}
	updateBorderColorPixmap();


	if (table->format().hasProperty(QTextFormat::FrameBorderStyle)) {
		QTextFrameFormat::BorderStyle style = table->format().borderStyle();
		int index = borderStyleComboBox->findData(style);
		if (index != -1) {
			borderStyleComboBox->setCurrentIndex(index);
		}
	} else {
		borderStyleComboBox->setCurrentIndex(0);
	}

	if (table->format().hasProperty(QTextFormat::FramePadding)) {
		paddingSpinBox->setValue(table->format().padding());
	} else {
		paddingSpinBox->setValue(0);
	}

	if (table->format().hasProperty(QTextFormat::BackgroundBrush)) {
		currentTableBGColor = table->format().background().color();
	} else {
		currentTableBGColor = QColor();
	}
	updateTableBGColorPixmap();
}

void TablePropertiesWidget::fillTableWidget() {
	for (int row = 0; row < tableColumnsCount + 1; row++) {
		QTableWidgetItem *newItem = 0;

		newItem = new QTableWidgetItem((row == 0 ? "Table" : QString("Column %1").arg(row)));
		tableWidget->setItem(row, 0, newItem);

		newItem = new QTableWidgetItem("");
		tableWidget->setItem(row, 1, newItem);

		newItem = new QTableWidgetItem("");
		tableWidget->setItem(row, 2, newItem);

		updateTableWidgetRowData(row);
	}
}

void TablePropertiesWidget::updateTableWidgetRowData(int row) {
	const QTextLength& currentItemConstraint =
			(row == 0 ? TableWidthConstraint : ColumnWidthConstraints[row - 1]);

	QString constraintType;
	QString valueString;

	switch (currentItemConstraint.type()) {
	case QTextLength::VariableLength:
		constraintType = "Minimal width";
		valueString = "";
		break;
	case QTextLength::FixedLength:
		constraintType = "Fixed width";
		valueString = QString("%1 px").arg(currentItemConstraint.rawValue());
		break;
	case QTextLength::PercentageLength:
		constraintType = "Percentage";
		valueString = QString("%1 %").arg(currentItemConstraint.rawValue());
		break;
	default:
		break;
	}

	QTableWidgetItem* item = tableWidget->item(row, 1);
	item->setText(constraintType);

	item = tableWidget->item(row, 2);
	item->setText(valueString);
}

void TablePropertiesWidget::addMissingColumnWidthConstraints() {
	for (int i = 0; i < tableColumnsCount; i++) {
		if (i >= ColumnWidthConstraints.size()) {
			ColumnWidthConstraints.push_back(QTextLength());
		}
	}
}

void TablePropertiesWidget::applyFormat() {
	QTextTableFormat TableFormat = table->format();

	Qt::Alignment alignment;
	if (centerAlignmentRadioButton->isChecked()) {
		alignment = Qt::AlignCenter;
	} else if (rightAlignmentRadioButton->isChecked()) {
		alignment = Qt::AlignRight;
	} else {
		alignment = Qt::AlignLeft;
	}
	TableFormat.setAlignment(alignment);

	TableFormat.setCellPadding(cellPaddingSpinBox->value());
	TableFormat.setCellSpacing(cellSpacingSpinBox->value());

	TableFormat.setWidth(TableWidthConstraint);
	TableFormat.setColumnWidthConstraints(ColumnWidthConstraints);

	if (borderWidthSpinBox->value() > 0) {
		TableFormat.setBorder((qreal)borderWidthSpinBox->value());
	} else {
		TableFormat.clearProperty(QTextFormat::FrameBorder);
	}

	if (currentBorderColor.isValid()) {
		QBrush brush = TableFormat.borderBrush();
		brush.setColor(currentBorderColor);
		TableFormat.setBorderBrush(brush);
	} else {
		TableFormat.clearProperty(QTextFormat::FrameBorderBrush);
	}


	if (borderStyleComboBox->currentIndex() > 0) {
		int styleValue = borderStyleComboBox->itemData(borderStyleComboBox->currentIndex()).toInt();
		QTextFrameFormat::BorderStyle style = (QTextFrameFormat::BorderStyle)styleValue;
		TableFormat.setBorderStyle(style);
	} else {
		TableFormat.clearProperty(QTextFormat::FrameBorderStyle);
	}

	TableFormat.setPadding(paddingSpinBox->value());

	if (currentTableBGColor.isValid()) {
		QBrush brush = TableFormat.borderBrush();
		brush.setColor(currentTableBGColor);
		TableFormat.setBackground(brush);
	} else {
		TableFormat.clearProperty(QTextFormat::BackgroundBrush);
	}

	table->setFormat(TableFormat);
}

void TablePropertiesWidget::accept() {
	applyFormat();
	QDialog::accept();
}

void TablePropertiesWidget::sl_ApplyButton_Clicked() {
	applyFormat();
}

void TablePropertiesWidget::sl_MinimalSizeRadioButton_Toggled() {
	if (doNotReactOnDataChange) {return;}

	if (!minimalSizeRadioButton->isChecked()) {return;}

	if (tableWidget->rowCount() == 0) {return;}

	const int row = tableWidget->currentRow();
	const QTextLength& currentItemConstraint =
			(row == 0 ? TableWidthConstraint : ColumnWidthConstraints[row - 1]);

	QTextLength newConstraint(QTextLength::VariableLength, currentItemConstraint.rawValue());

	if (row == 0) {
		TableWidthConstraint = newConstraint;
	} else {
		ColumnWidthConstraints[row - 1] = newConstraint;
	}

	updateTableWidgetRowData(row);
}

void TablePropertiesWidget::sl_FixedSizeRadioButton_Toggled() {
	widthLineEdit->setEnabled(fixedSizeRadioButton->isChecked());

	if (doNotReactOnDataChange) {return;}

	if (!fixedSizeRadioButton->isChecked()) {return;}

	if (tableWidget->rowCount() == 0) {return;}

	const int row = tableWidget->currentRow();

	bool convertionSuccess = false;
	int value = widthLineEdit->text().toInt(&convertionSuccess, 10);
	if (!convertionSuccess) {value = 0;}

	QTextLength newConstraint(QTextLength::FixedLength, (qreal)value);

	if (row == 0) {
		TableWidthConstraint = newConstraint;
	} else {
		ColumnWidthConstraints[row - 1] = newConstraint;
	}

	updateTableWidgetRowData(row);
}

void TablePropertiesWidget::sl_PercentageSizeRadioButton_Toggled() {
	widthSpinBox->setEnabled(percentageSizeRadioButton->isChecked());

	if (doNotReactOnDataChange) {return;}

	if (!percentageSizeRadioButton->isChecked()) {return;}

	if (tableWidget->rowCount() == 0) {return;}

	const int row = tableWidget->currentRow();

	int value = widthSpinBox->value();

	QTextLength newConstraint(QTextLength::PercentageLength, (qreal)value);

	if (row == 0) {
		TableWidthConstraint = newConstraint;
	} else {
		ColumnWidthConstraints[row - 1] = newConstraint;
	}

	updateTableWidgetRowData(row);
}

void TablePropertiesWidget::sl_WidthLineEdit_TextEdited(QString text) {
	if (doNotReactOnDataChange) {return;}

	bool convertionSuccess = false;
	int value = text.toInt(&convertionSuccess, 10);
	if (!convertionSuccess) {value = 0;}

	if (tableWidget->rowCount() == 0) {return;}

	const int row = tableWidget->currentRow();
	const QTextLength& currentItemConstraint =
			(row == 0 ? TableWidthConstraint : ColumnWidthConstraints[row - 1]);

	QTextLength newConstraint(currentItemConstraint.type(), (qreal)value);

	if (row == 0) {
		TableWidthConstraint = newConstraint;
	} else {
		ColumnWidthConstraints[row - 1] = newConstraint;
	}

	updateTableWidgetRowData(row);
}

void TablePropertiesWidget::sl_WidthSpinBox_ValueChanged(int value) {
	if (doNotReactOnDataChange) {return;}

	if (tableWidget->rowCount() == 0) {return;}

	const int row = tableWidget->currentRow();
	const QTextLength& currentItemConstraint =
			(row == 0 ? TableWidthConstraint : ColumnWidthConstraints[row - 1]);

	QTextLength newConstraint(currentItemConstraint.type(), (qreal)value);

	if (row == 0) {
		TableWidthConstraint = newConstraint;
	} else {
		ColumnWidthConstraints[row - 1] = newConstraint;
	}

	updateTableWidgetRowData(row);
}

void TablePropertiesWidget::sl_TableWidget_ItemSelectionChanged() {
	if (tableWidget->rowCount() == 0) {return;}

	const int row = tableWidget->currentRow();
	const QTextLength& currentItemConstraint =
			(row == 0 ? TableWidthConstraint : ColumnWidthConstraints[row - 1]);

	doNotReactOnDataChange = true;

	switch (currentItemConstraint.type()) {
	case QTextLength::VariableLength:
		minimalSizeRadioButton->setChecked(true);
		break;

	case QTextLength::FixedLength:
		fixedSizeRadioButton->setChecked(true);

		break;

	case QTextLength::PercentageLength:
		percentageSizeRadioButton->setChecked(true);
		break;

	default:
		break;
	}

	widthLineEdit->setText(QString("%1").arg(currentItemConstraint.rawValue()));
	widthSpinBox->setValue((int)currentItemConstraint.rawValue());

	doNotReactOnDataChange = false;
}

void TablePropertiesWidget::sl_SelectBorderColorButton_Pressed() {
	QColor newColor = QColorDialog::getColor(currentBorderColor, this);
	if (newColor.isValid()) {
		currentBorderColor = newColor;
	}

	updateBorderColorPixmap();
}

void TablePropertiesWidget::sl_SelectTableBGColorButton_Pressed() {
	QColor newColor = QColorDialog::getColor(currentTableBGColor, this, QString(), QColorDialog::ShowAlphaChannel);
	currentTableBGColor = newColor;

	updateTableBGColorPixmap();
}

void TablePropertiesWidget::sl_ResetTableBGColorButton_Pressed() {
	currentTableBGColor = QColor();

	updateTableBGColorPixmap();
}

void TablePropertiesWidget::updateBorderColorPixmap() {
	paintPixmap(borderColorPixmap, currentBorderColor);

	borderColorDisplayLabel->setPixmap(borderColorPixmap);
}

void TablePropertiesWidget::updateTableBGColorPixmap() {
	paintPixmap(tableBGColorPixmap, currentTableBGColor);

	tableBGColorDisplayLabel->setPixmap(tableBGColorPixmap);
}

void TablePropertiesWidget::paintPixmap(QPixmap& pixmap, QColor color) {
	const QRect fillRect(QPoint(0, 0), pixmap.size());
	const QRect borderRect(0, 0, pixmap.width() - 1, pixmap.height() - 1);

	QBrush brush;
	brush.setStyle(Qt::SolidPattern);

	QPainter painter;

	painter.begin(&pixmap);
		// Draw white background
		brush.setColor(Qt::white);
		painter.setPen(Qt::NoPen);
		painter.setBrush(brush);
		painter.drawRect(fillRect);

		if (color.isValid()) {
			brush.setColor(color);
			painter.setPen(Qt::NoPen);
			painter.setBrush(brush);
			painter.drawRect(fillRect);
		} else {
			// Draw "X"
			painter.setPen(Qt::black);
			painter.setBrush(Qt::NoBrush);
			painter.setRenderHint(QPainter::Antialiasing, true);
			painter.drawLine(0, 0, pixmap.width(), pixmap.height());
			painter.drawLine(0, pixmap.height(), pixmap.width(), 0);
			painter.setRenderHint(QPainter::Antialiasing, false);
		}

		// Draw border
		painter.setPen(Qt::black);
		painter.setBrush(Qt::NoBrush);
		painter.drawRect(borderRect);
	painter.end();
}
