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

#ifndef SIZEEDITWIDGET_H
#define SIZEEDITWIDGET_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QToolButton>
#include <QDialogButtonBox>

namespace qNotesManager {
	class SizeEditWidget : public QDialog {
		Q_OBJECT
	private:
		QDialogButtonBox* buttonBox;

		QLabel* widthLabel;
		QSlider* widthSlider;
		QSpinBox* widthSpinBox;

		QLabel* heightLabel;
		QSlider* heightSlider;
		QSpinBox* heightSpinBox;

		QToolButton* keepRationButton;

		static const int maximalImageSize = 1000;
		static const int minimalImageSize = 10;

		QSize imageCurrentSize;
		QSize imageOriginalSize;
		double imageOriginalRatio;

	public:
		explicit SizeEditWidget(QWidget *parent = nullptr);
		void SetData(const QSize&, const QSize&);

		QSize NewSize;

		virtual void accept();

	private slots:
		void sl_ButtonBox_Clicked(QAbstractButton* button);

		void sl_WidthSlider_ValueChanged(int);
		void sl_HeightSlider_ValueChanged(int);

		void sl_WidthSpinBox_ValueChanged(int);
		void sl_HeightSpinBox_ValueChanged(int);

		void sl_KeepRatioButton_Toggled();

	};
}

#endif // SIZEEDITWIDGET_H
