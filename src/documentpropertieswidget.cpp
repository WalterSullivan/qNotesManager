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

#include "documentpropertieswidget.h"

#include "document.h"
#include "compressor.h"
#include "cipherer.h"
#include "global.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QToolTip>
#include <QPushButton>

using namespace qNotesManager;

DocumentPropertiesWidget::DocumentPropertiesWidget(QWidget *parent) : QDialog(parent) {
	filenameCaptionLabel = new QLabel("File name:", this);
	filenameLabel = new QLabel(this);
	creationDateCaptionLabel = new QLabel("Creation date:", this);
	creationDateLabel = new QLabel(this);
	modificationDateCaptionLabel = new QLabel("Modification date:", this);
	modificationDateLabel = new QLabel(this);

	encryptionGroupBox = new QGroupBox("Encryption", this);
	encryptionGroupBox->setEnabled(false);
	useEncryptionCheckbox = new QCheckBox("Encrypt document file", this);
	QObject::connect(useEncryptionCheckbox, SIGNAL(stateChanged(int)),
					 this, SLOT(sl_UseEncryptionCB_StateChanged(int)));
	passwordLabel = new QLabel("Password:", this);
	passwordLineEdit = new QLineEdit(this);
	passwordLineEdit->setEchoMode(QLineEdit::Password);
	passwordLineEdit->setToolTip("Enter a password for your document."
								 "Note that if you forget the password you will be unable to "
								 "open your encrypted document.");
	encryptionAlgLabel = new QLabel("Encryption algorithm:", this);
	encryptionAlg = new QComboBox(this);
	encryptionAlg->setToolTip("Select encryption algorithm. If you don't know what it is, just "
							  "leave default value.");

	Cipherer c;
	const QList<int> cipherIDs = c.GetAvaliableCipherIDs();
	foreach (const int id, cipherIDs) {
		const QString name = c.GetCipherName(id);
		encryptionAlg->addItem(name, id);
	}

	// Buttons
	buttonBox = new QDialogButtonBox(this);
	QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(sl_ButtonBox_Clicked(QAbstractButton*)));
	buttonBox->addButton(QDialogButtonBox::Ok)->setDefault(true);
	buttonBox->addButton(QDialogButtonBox::Cancel)->setAutoDefault(false);

	QGridLayout* encryptionLayout = new QGridLayout();
	encryptionLayout->addWidget(passwordLabel, 0, 0);
	encryptionLayout->addWidget(passwordLineEdit, 0, 1);
	encryptionLayout->addWidget(encryptionAlgLabel, 1, 0);
	encryptionLayout->addWidget(encryptionAlg, 1, 1);
	encryptionGroupBox->setLayout(encryptionLayout);

	QGridLayout* gridLayout = new QGridLayout();
	gridLayout->addWidget(filenameCaptionLabel, 0, 0);
	gridLayout->addWidget(filenameLabel, 0, 1);
	gridLayout->addWidget(creationDateCaptionLabel, 1, 0);
	gridLayout->addWidget(creationDateLabel, 1, 1);
	gridLayout->addWidget(modificationDateCaptionLabel, 2, 0);
	gridLayout->addWidget(modificationDateLabel, 2, 1);
	gridLayout->addWidget(useEncryptionCheckbox, 3, 0);
	gridLayout->addWidget(encryptionGroupBox, 3, 1, 2, 1);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(gridLayout);
	mainLayout->addStretch();
	mainLayout->addWidget(buttonBox);

	setLayout(mainLayout);
	setWindowIcon(QIcon(":/gui/property"));
	setWindowTitle("Document properties");
	resize(400, height());
}

void DocumentPropertiesWidget::SetDocument(Document* d) {
	if (d == nullptr) {
		WARNING("Null pointer recieved");
		return;
	}

	QString fname = d->GetFilename().isEmpty() ? "<unsaved>" : d->GetFilename();
	filenameLabel->setText(fname);
	creationDateLabel->setText(d->GetCreationDate().toString(Qt::SystemLocaleLongDate));
	modificationDateLabel->setText(d->GetModificationDate().toString(Qt::SystemLocaleLongDate));

	quint8 cipherID = d->GetCipherID();
	if (cipherID == 0) {
		useEncryptionCheckbox->setChecked(false);
		encryptionAlg->setCurrentIndex(-1);
		passwordLineEdit->setText(QString());
	} else {
		useEncryptionCheckbox->setChecked(true);
		int index = encryptionAlg->findData(cipherID, Qt::UserRole);
		if (index == -1) {
			WARNING("Situable index not found");
			encryptionAlg->setCurrentIndex(0);
		} else {
			encryptionAlg->setCurrentIndex(index);
		}
		passwordLineEdit->setText(d->GetPassword());

	}

	currentDocument = d;
}

void DocumentPropertiesWidget::sl_UseEncryptionCB_StateChanged(int) {
	bool enabled = useEncryptionCheckbox->isChecked();
	encryptionGroupBox->setEnabled(enabled);
	if (encryptionAlg->currentIndex() == -1) {encryptionAlg->setCurrentIndex(0);}
}

void DocumentPropertiesWidget::accept() {
	if (useEncryptionCheckbox->isChecked()) {
		if (encryptionAlg->currentIndex() == -1) {
			encryptionAlg->setFocus(Qt::OtherFocusReason);
			encryptionAlg->showPopup();
			QToolTip::showText(encryptionAlg->mapToGlobal(QPoint(0,0)),
							   "Select encryption algorithm", encryptionAlg);
			return;
		}
		if (passwordLineEdit->text().isEmpty()) {
			passwordLineEdit->setFocus(Qt::OtherFocusReason);
			QToolTip::showText(passwordLineEdit->mapToGlobal(QPoint(0,0)),
							   "Enter password", passwordLineEdit);
			return;
		}
	}

	quint8 cipherID = 0;
	if (useEncryptionCheckbox->isChecked()) {
		QVariant data = encryptionAlg->itemData(encryptionAlg->currentIndex(), Qt::UserRole);
#if QT_VERSION >= 0x050000
		cipherID = data.value<quint8>();
#else
		cipherID = qVariantValue<quint8>(data);
#endif
	}
	if (currentDocument->GetCipherID() != cipherID ||
		currentDocument->GetPassword() != passwordLineEdit->text()) {
		currentDocument->SetCipherData(cipherID, passwordLineEdit->text());
	}

	QDialog::accept();
}

void DocumentPropertiesWidget::reject() {
	currentDocument = nullptr;

	QDialog::reject();
}

void DocumentPropertiesWidget::sl_ButtonBox_Clicked(QAbstractButton* button) {
	QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(button);

	switch(role) {
		case QDialogButtonBox::AcceptRole:
			accept();
			break;
		case QDialogButtonBox::RejectRole:
			reject();
			break;
		default:
			reject();
	}
}
