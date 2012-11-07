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

#ifndef FOLDERITEMPROPERTIESWIDGET_H
#define FOLDERITEMPROPERTIESWIDGET_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

namespace qNotesManager {
	class AbstractFolderItem;
	class CustomIconsListWidget;

	class FolderItemPropertiesWidget : public QDialog {
	Q_OBJECT

	private:
		AbstractFolderItem* itemToEdit;
		QLabel* creationDateLabel;
		QLabel* creationDateLabelD;

		QLabel* modificationDateLabel;
		QLabel* modificationDateLabelD;

		QLabel*			nameLabel;
		QLineEdit*		nameLineEdit;

		QGroupBox*		iconGroupBox;
		QLabel*			iconLabel;
		QPushButton*	chooseIconButton;
		QPushButton*	resetIconToDefaultButton;
		QPushButton*	setDefaultIconButton;

		QPushButton*	okButton;
		QPushButton*	cancelButton;

		CustomIconsListWidget* customIconsWidget;

		QString selectedIconKey;

	public:
		explicit FolderItemPropertiesWidget(QWidget *parent);

		void SetFolderItem(AbstractFolderItem* item);

	signals:

	public slots:
	private slots:
		void sl_OKButton_Clicked();
		void sl_CancelButton_Clicked();
		void sl_ChooseIconButton_Clicked();
		void sl_ResetIconToDefaultButton_Clicked();
		void sl_SetDefaultIconButton_Clicked();

	};
}

#endif // FOLDERITEMPROPERTIESWIDGET_H
