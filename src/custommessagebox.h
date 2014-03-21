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

#ifndef CUSTOMMESSAGEBOX_H
#define CUSTOMMESSAGEBOX_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>

namespace qNotesManager {
	class CustomMessageBox : public QDialog {
	Q_OBJECT
	private:
		QLabel* captionLabel;
		QLabel* textLabel;
		QLabel* imageLabel;
		QDialogButtonBox* buttonsBox;
		QAbstractButton* clickedButton;

		QVBoxLayout* layout;

		QPixmap standardIcon(QMessageBox::Icon icon);

	public:
		explicit CustomMessageBox(const QString& text,
							const QString& caption = QString(),
							QMessageBox::Icon Icon = QMessageBox::NoIcon,
							QMessageBox::StandardButtons buttons = QMessageBox::Ok,
							QMessageBox::StandardButton defaultButton = QMessageBox::NoButton,
							QWidget* parent = 0);

		QMessageBox::StandardButton show();

		void AddCustomWidget(QWidget*);

	private slots:
		void sl_ButtonBox_ButtonClicked(QAbstractButton*);

	};
}

#endif // CUSTOMMESSAGEBOX_H
