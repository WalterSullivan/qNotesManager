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

#include "edittablewidthconstraintswidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>

using namespace qNotesManager;

EditTableWidthConstraintsWidget::EditTableWidthConstraintsWidget(const QTextTableFormat& format, int columnsCount, QWidget *parent) :
	QDialog(parent)
{
	TableWidthConstraint = format.width();
	ColumnWidthConstraints = format.columnWidthConstraints();
	tableColumnsCount = columnsCount;
	addMissingColumnWidthConstraints();

	doNotReactOnDataChange = false;

	tableWidget = new QTableWidget();
	tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	tableWidget->setColumnCount(3);
	tableWidget->setRowCount(columnsCount + 1);
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

	okButton = new QPushButton("OK");
	QObject::connect(okButton, SIGNAL(clicked()),
					 this, SLOT(accept()));
	cancelButton = new QPushButton("Cancel");
	QObject::connect(cancelButton, SIGNAL(clicked()),
					 this, SLOT(reject()));
	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);


	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(sizeLayout);
	mainLayout->addLayout(buttonsLayout);

	setLayout(mainLayout);
	setWindowTitle("Edit table width");

	fillTableWidget();
	tableWidget->resizeColumnsToContents();
	tableWidget->setCurrentCell(0, 0);
}

void EditTableWidthConstraintsWidget::fillTableWidget() {
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

void EditTableWidthConstraintsWidget::updateTableWidgetRowData(int row) {
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

void EditTableWidthConstraintsWidget::addMissingColumnWidthConstraints() {
	for (int i = 0; i < tableColumnsCount; i++) {
		if (i >= ColumnWidthConstraints.size()) {
			ColumnWidthConstraints.push_back(QTextLength());
		}
	}
}

void EditTableWidthConstraintsWidget::sl_MinimalSizeRadioButton_Toggled() {
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

void EditTableWidthConstraintsWidget::sl_FixedSizeRadioButton_Toggled() {
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

void EditTableWidthConstraintsWidget::sl_PercentageSizeRadioButton_Toggled() {
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

void EditTableWidthConstraintsWidget::sl_WidthLineEdit_TextEdited(QString text) {
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

void EditTableWidthConstraintsWidget::sl_WidthSpinBox_ValueChanged(int value) {
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

void EditTableWidthConstraintsWidget::sl_TableWidget_ItemSelectionChanged() {
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

