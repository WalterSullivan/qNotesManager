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

#include <QGridLayout>
#include <QHBoxLayout>
#include <QToolTip>

#include "document.h"
#include "compressor.h"
#include "cipherer.h"

using namespace qNotesManager;

DocumentPropertiesWidget::DocumentPropertiesWidget(QWidget *parent) : QDialog(parent) {
	filenameCaptionLabel = new QLabel("File name:", this);
	filenameLabel = new QLabel(this);
	creationDateCaptionLabel = new QLabel("Creation date:", this);
	creationDateLabel = new QLabel(this);
	modificationDateCaptionLabel = new QLabel("Modification date:", this);
	modificationDateLabel = new QLabel(this);
	useCompressionCheckbox = new QCheckBox("Compress document file", this);
	QObject::connect(useCompressionCheckbox, SIGNAL(stateChanged(int)),
					 this, SLOT(sl_UseCompressionCB_StateChanged(int)));
	compressionLevelLabel = new QLabel("Compression level:", this);
	compressionLevelLabel->setEnabled(false);
	compressionLevel = new QComboBox(this);
	compressionLevel->setToolTip("Set compression level for document. "
								 "The more level is the less file size will be.");
	compressionLevel->setEnabled(false);

	for (quint8 level = Compressor::MinimumLevel; level <= Compressor::MaximumLevel; level++) {
		compressionLevel->addItem(QString::number(level), level);
	}

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

	okButton = new QPushButton("OK", this);
	QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(sl_OKButton_Clicked()));
	cancelButton = new QPushButton("Cancel", this);
	QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(sl_CancelButton_Clicked()));

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
	gridLayout->addWidget(useCompressionCheckbox, 3, 0);
	gridLayout->addWidget(compressionLevelLabel, 3, 1);
	gridLayout->addWidget(compressionLevel, 4, 1);
	gridLayout->addWidget(useEncryptionCheckbox, 5, 0);
	gridLayout->addWidget(encryptionGroupBox, 5, 1, 2, 1);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(gridLayout);
	mainLayout->addLayout(buttonsLayout);

	setLayout(mainLayout);
	setWindowIcon(QIcon(":/gui/property"));
	setWindowTitle("Document properties");
}

void DocumentPropertiesWidget::SetDocument(Document* d) {
	Q_ASSERT(d != 0);

	QString fname = d->GetFilename().isEmpty() ? "<unsaved>" : d->GetFilename();
	filenameLabel->setText(fname);
	creationDateLabel->setText(d->GetCreationDate().toString(Qt::SystemLocaleLongDate));
	modificationDateLabel->setText(d->GetModificationDate().toString(Qt::SystemLocaleLongDate));
	quint8 compLevel = d->GetCompressionLevel();
	if (compLevel == 0) {
		useCompressionCheckbox->setChecked(false);
		compressionLevel->setCurrentIndex(-1);
	} else {
		useCompressionCheckbox->setChecked(true);
		int index = compressionLevel->findData(compLevel, Qt::UserRole);
		Q_ASSERT(index != -1);
		compressionLevel->setCurrentIndex(index);
	}

	quint8 cipherID = d->GetCipherID();
	if (cipherID == 0) {
		useEncryptionCheckbox->setChecked(false);
		encryptionAlg->setCurrentIndex(-1);
		passwordLineEdit->setText(QString());
	} else {
		useEncryptionCheckbox->setChecked(true);
		int index = encryptionAlg->findData(cipherID, Qt::UserRole);
		Q_ASSERT(index != -1);
		encryptionAlg->setCurrentIndex(index);
		passwordLineEdit->setText(d->GetPassword());
	}

	currentDocument = d;
}

void DocumentPropertiesWidget::sl_UseCompressionCB_StateChanged(int) {
	bool enabled = useCompressionCheckbox->isChecked();
	compressionLevel->setEnabled(enabled);
	compressionLevelLabel->setEnabled(enabled);
	if (compressionLevel->currentIndex() == -1) {compressionLevel->setCurrentIndex(0);}
}

void DocumentPropertiesWidget::sl_UseEncryptionCB_StateChanged(int) {
	bool enabled = useEncryptionCheckbox->isChecked();
	encryptionGroupBox->setEnabled(enabled);
	if (encryptionAlg->currentIndex() == -1) {encryptionAlg->setCurrentIndex(0);}
}

void DocumentPropertiesWidget::sl_OKButton_Clicked() {
	if (useCompressionCheckbox->isChecked() &&
		compressionLevel->currentIndex() == -1) {
		compressionLevel->setFocus(Qt::OtherFocusReason);
		compressionLevel->showPopup();
		QToolTip::showText(compressionLevel->mapToGlobal(QPoint(0,0)),
						   "Select compression level", compressionLevel);
		return;
	}
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

	quint8 compLevel = 0;
	if (useCompressionCheckbox->isChecked()) {
		QVariant data = compressionLevel->itemData(compressionLevel->currentIndex(), Qt::UserRole);
		compLevel = qVariantValue<quint8>(data);
	}
	if (currentDocument->GetCompressionLevel() != compLevel) {
		currentDocument->SetCompressionLevel(compLevel);
	}

	quint8 cipherID = 0;
	if (useEncryptionCheckbox->isChecked()) {
		QVariant data = encryptionAlg->itemData(encryptionAlg->currentIndex(), Qt::UserRole);
		cipherID = qVariantValue<quint8>(data);
	}
	if (currentDocument->GetCipherID() != cipherID ||
		currentDocument->GetPassword() != passwordLineEdit->text()) {
		currentDocument->SetCipherData(cipherID, passwordLineEdit->text());
	}

	close();
}

void DocumentPropertiesWidget::sl_CancelButton_Clicked() {
	close();
}
