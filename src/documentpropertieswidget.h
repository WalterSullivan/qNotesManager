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

#ifndef DOCUMENTPROPERTIESWIDGET_H
#define DOCUMENTPROPERTIESWIDGET_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QPushButton>

namespace qNotesManager {
	class Document;

	class DocumentPropertiesWidget : public QDialog {
	Q_OBJECT
	private:
		QLabel* filenameCaptionLabel;
		QLabel* filenameLabel;
		QLabel* creationDateCaptionLabel;
		QLabel* creationDateLabel;
		QLabel* modificationDateCaptionLabel;
		QLabel* modificationDateLabel;
		QGroupBox* encryptionGroupBox;
		QCheckBox* useEncryptionCheckbox;
		QLabel* passwordLabel;
		QLineEdit* passwordLineEdit;
		QLabel* encryptionAlgLabel;
		QComboBox* encryptionAlg;

		QPushButton* okButton;
		QPushButton* cancelButton;

		Document* currentDocument;

	public:
		explicit DocumentPropertiesWidget(QWidget *parent = 0);
		void SetDocument(Document* d);

	private slots:
		void sl_UseEncryptionCB_StateChanged(int = 0);
		void sl_OKButton_Clicked();
		void sl_CancelButton_Clicked();

	};
}

#endif // DOCUMENTPROPERTIESWIDGET_H
