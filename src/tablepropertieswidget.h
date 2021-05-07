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

#ifndef TABLEPROPERTIESWIDGET_H
#define TABLEPROPERTIESWIDGET_H

#include <QDialog>

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QRadioButton>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTabWidget>
#include <QComboBox>
#include <QTextTable>
#include <QTextTableFormat>
#include <QDialogButtonBox>

namespace qNotesManager {
	class TablePropertiesWidget : public QDialog {
		Q_OBJECT
	private:
		QDialogButtonBox* buttonBox;

		QTabWidget* tabWidget;

		// Table tab
		QGroupBox* alignmentGroupBox;
		QRadioButton* leftAlignmentRadioButton;
		QRadioButton* centerAlignmentRadioButton;
		QRadioButton* rightAlignmentRadioButton;
		QLabel* cellPaddingLabel;
		QDoubleSpinBox* cellPaddingSpinBox;
		QLabel* cellSpacingLabel;
		QDoubleSpinBox* cellSpacingSpinBox;

		QGroupBox* borderGroupBox;
		QLabel* borderWidthLabel;
		QDoubleSpinBox* borderWidthSpinBox;

		QLabel* borderColorLabel;
		QLabel* borderColorDisplayLabel;
		QPixmap borderColorPixmap;
		QColor currentBorderColor;
		QPushButton* selectBorderColorButton;
		void updateBorderColorPixmap();

		QLabel* borderStyleLabel;
		QComboBox* borderStyleComboBox;

		QLabel* paddingLabel;
		QDoubleSpinBox* paddingSpinBox;

		QLabel* tableBGColorLabel;
		QLabel* tableBGColorDisplayLabel;
		QPixmap tableBGColorPixmap;
		QColor currentTableBGColor;
		QPushButton* selectTableBGColorButton;
		QPushButton* resetTableBGColorButton;
		void updateTableBGColorPixmap();

		// Columns' width tab
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
		QTextLength TableWidthConstraint;
		QVector<QTextLength> ColumnWidthConstraints;

		QTextTable* table;

		void loadData();
		void applyFormat();
		void paintPixmap(QPixmap&, QColor);

	public:
		explicit TablePropertiesWidget(QTextTable* t, QWidget* parent);
		virtual void accept();

	private slots:
		void sl_ButtonBox_Clicked(QAbstractButton* button);

		void sl_MinimalSizeRadioButton_Toggled();
		void sl_FixedSizeRadioButton_Toggled();
		void sl_PercentageSizeRadioButton_Toggled();
		void sl_WidthLineEdit_TextEdited(QString);
		void sl_WidthSpinBox_ValueChanged(int);
		void sl_TableWidget_ItemSelectionChanged();
		void sl_SelectBorderColorButton_Pressed();
		void sl_SelectTableBGColorButton_Pressed();
		void sl_ResetTableBGColorButton_Pressed();
	};
}

#endif // TABLEPROPERTIESWIDGET_H
