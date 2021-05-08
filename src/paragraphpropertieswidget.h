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

#ifndef PARAGRAPHPROPERTIESWIDGET_H
#define PARAGRAPHPROPERTIESWIDGET_H

#include <QDialog>

#include <QRadioButton>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QTextBlockFormat>
#include <QTextCursor>

namespace qNotesManager {
	class ParagraphPropertiesWidget : public QDialog {
		Q_OBJECT
	private:
		QDialogButtonBox* buttonBox;

		QGroupBox* alignmentGroupBox;
		QRadioButton* leftAlignmentRadioButton;
		QRadioButton* centerAlignmentRadioButton;
		QRadioButton* rightAlignmentRadioButton;
		QRadioButton* justifyAlignmentRadioButton;

		QLabel* textIndentLabel;
		QSpinBox* textIndentSpinBox;

		QCheckBox* nonBreakableLinesCheckbox;

		QLabel* leftMarginLabel;
		QSpinBox* leftMarginSpinBox;
		QLabel* rightMarginLabel;
		QSpinBox* rightMarginSpinBox;
		QLabel* topMarginLabel;
		QSpinBox* topMarginSpinBox;
		QLabel* bottomMarginLabel;
		QSpinBox* bottomMarginSpinBox;

#if QT_VERSION >= 0x040800
		QGroupBox* lineHeightGroupBox;
		QLabel* lineHeightLabel;
		QSpinBox* lineHeightSpinBox;
		QLabel* lineHeightTypeLabel;
		QComboBox* lineHeightTypeComboBox;
#endif

		QTextCursor cursor;

		void loadData();
		bool applyFormat();

	public:
		explicit ParagraphPropertiesWidget(QWidget *parent = 0);
		virtual void accept();

		void SetCursor(QTextCursor c);
	signals:

	public slots:
	private slots:
		void sl_ButtonBox_Clicked(QAbstractButton* button);
#if QT_VERSION >= 0x040800
		void sl_LineHeightTypeComboBox_CurrentIndexChanged(int);
#endif
	};
}

#endif // PARAGRAPHPROPERTIESWIDGET_H
