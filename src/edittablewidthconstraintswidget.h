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

#ifndef EDITTABLEWIDTHCONSTRAINTSWIDGET_H
#define EDITTABLEWIDTHCONSTRAINTSWIDGET_H

#include <QDialog>
#include <QTextTableFormat>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QRadioButton>
#include <QGroupBox>
#include <QSpinBox>

namespace qNotesManager {
	class EditTableWidthConstraintsWidget : public QDialog {
	Q_OBJECT
	private:
		QPushButton* okButton;
		QPushButton* cancelButton;

		QTableWidget* tableWidget;

		QGroupBox* groupBox;
		QRadioButton* minimalSizeRadioButton;
		QRadioButton* fixedSizeRadioButton;
		QRadioButton* percentageSizeRadioButton;
		QLineEdit* widthLineEdit;
		QSpinBox* widthSpinBox;

		void fillTableWidget();
		void addMissingColumnWidthConstraints();
		void updateTableWidgetRowData(int row);

		int tableColumnsCount;
		bool doNotReactOnDataChange;

	public:
		explicit EditTableWidthConstraintsWidget(const QTextTableFormat& format, int columnsCount, QWidget *parent = 0);

		QTextLength TableWidthConstraint;
		QVector<QTextLength> ColumnWidthConstraints;

	private slots:
		void sl_MinimalSizeRadioButton_Toggled();
		void sl_FixedSizeRadioButton_Toggled();
		void sl_PercentageSizeRadioButton_Toggled();

		void sl_WidthLineEdit_TextEdited(QString);
		void sl_WidthSpinBox_ValueChanged(int);

		void sl_TableWidget_ItemSelectionChanged();
	};
}

#endif // EDITTABLEWIDTHCONSTRAINTSWIDGET_H
