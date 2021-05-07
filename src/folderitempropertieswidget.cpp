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

#include "folderitempropertieswidget.h"

#include "abstractfolderitem.h"
#include "folder.h"
#include "note.h"
#include "customiconslistwidget.h"
#include "application.h"
#include "document.h"
#include "global.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>

using namespace qNotesManager;

FolderItemPropertiesWidget::FolderItemPropertiesWidget(QWidget *parent) : QDialog(parent) {
	customIconsWidget = nullptr;

	creationDateLabel = new QLabel("Creation date:", this);
	creationDateLabelD = new QLabel("", this);

	modificationDateLabel = new QLabel("Modification date:", this);
	modificationDateLabelD = new QLabel("", this);

	nameLabel = new QLabel("Name:", this);
	nameLineEdit = new QLineEdit();


	iconLabel = new QLabel();

	chooseIconButton = new QPushButton("Change", this);
	QObject::connect(chooseIconButton, SIGNAL(clicked()),
					 this, SLOT(sl_ChooseIconButton_Clicked()));

	resetIconToDefaultButton = new QPushButton("Reset to default", this);
	QObject::connect(resetIconToDefaultButton, SIGNAL(clicked()),
					 this, SLOT(sl_ResetIconToDefaultButton_Clicked()));

	setDefaultIconButton = new QPushButton("Set this icon as default", this);
	QObject::connect(setDefaultIconButton, SIGNAL(clicked()),
					 this, SLOT(sl_SetDefaultIconButton_Clicked()));

	iconGroupBox = new QGroupBox(this);
	iconGroupBox->setTitle("Icon");
	QGridLayout* iconLayout = new QGridLayout();
	iconLayout->addWidget(iconLabel, 0, 0, 3, 1, Qt::AlignCenter);
	iconLayout->addWidget(chooseIconButton, 0, 1);
	iconLayout->addWidget(resetIconToDefaultButton, 1, 1);
	iconLayout->addWidget(setDefaultIconButton, 2, 1);

	iconGroupBox->setLayout(iconLayout);

	// Buttons
	buttonBox = new QDialogButtonBox(this);
	QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(sl_ButtonBox_Clicked(QAbstractButton*)));
	buttonBox->addButton(QDialogButtonBox::Ok)->setDefault(true);
	buttonBox->addButton(QDialogButtonBox::Cancel)->setAutoDefault(false);

	QGridLayout* controlsLayout = new QGridLayout();
	controlsLayout->addWidget(creationDateLabel, 0, 0);
	controlsLayout->addWidget(creationDateLabelD, 0, 1);

	controlsLayout->addWidget(modificationDateLabel, 1, 0);
	controlsLayout->addWidget(modificationDateLabelD, 1, 1);

	controlsLayout->addWidget(nameLabel, 2, 0);
	controlsLayout->addWidget(nameLineEdit, 2, 1);

	controlsLayout->addWidget(iconGroupBox, 3, 0, 1, 2);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(controlsLayout);
	mainLayout->addStretch();
	mainLayout->addWidget(buttonBox);

	setLayout(mainLayout);
	setWindowTitle("Edit properties");
	setWindowIcon(QIcon(":/gui/property"));
	selectedIconKey = "";
	resize(400, height());
}

void FolderItemPropertiesWidget::SetFolderItem(AbstractFolderItem* item) {
	if (item == nullptr) {
		WARNING("Null pointer recieved");
		return;
	}

	itemToEdit = item;

	if (item->GetItemType() == AbstractFolderItem::Type_Folder) {
		Folder* f = dynamic_cast<Folder*>(item);
		creationDateLabelD->setText(f->GetCreationDate().toString(Qt::SystemLocaleShortDate));
		modificationDateLabelD->setText(f->GetModificationDate().toString(Qt::SystemLocaleShortDate));
		nameLineEdit->setText(f->GetName());
		iconLabel->setPixmap(f->GetIcon());
		selectedIconKey = f->GetIconID();
		setWindowTitle(QString("Edit properties for \"%1\"").arg(f->GetName()));

		nameLineEdit->setEnabled(!f->IsLocked());
		chooseIconButton->setEnabled(!f->IsLocked());
		resetIconToDefaultButton->setEnabled(!f->IsLocked());

	} else if (item->GetItemType() == AbstractFolderItem::Type_Note) {
		Note* n = dynamic_cast<Note*>(item);
		creationDateLabelD->setText(n->GetCreationDate().toString(Qt::SystemLocaleShortDate));
		modificationDateLabelD->setText(n->GetModificationDate().toString(Qt::SystemLocaleShortDate));
		nameLineEdit->setText(n->GetName());
		iconLabel->setPixmap(n->GetIcon());
		selectedIconKey = n->GetIconID();
		setWindowTitle(QString("Edit properties for \"%1\"").arg(n->GetName()));

		nameLineEdit->setEnabled(!n->IsLocked());
		chooseIconButton->setEnabled(!n->IsLocked());
		resetIconToDefaultButton->setEnabled(!n->IsLocked());
	}

	nameLineEdit->setFocus();
	nameLineEdit->selectAll();
}

void FolderItemPropertiesWidget::accept() {
	if (itemToEdit == nullptr) {
		WARNING("Null pointer recieved");
		QDialog::reject();
	}

	if (nameLineEdit->text().isEmpty()) {nameLineEdit->setText("Noname");} // ?

	if (itemToEdit->GetItemType() == AbstractFolderItem::Type_Folder) {
		Folder* f = dynamic_cast<Folder*>(itemToEdit);
		f->SetName(nameLineEdit->text());
		f->SetIconID(selectedIconKey);
	} else if (itemToEdit->GetItemType() == AbstractFolderItem::Type_Note) {
		Note* n = dynamic_cast<Note*>(itemToEdit);
		n->SetName(nameLineEdit->text());
		n->SetIconID(selectedIconKey);
	}

	itemToEdit = nullptr;

	QDialog::accept();
}

void FolderItemPropertiesWidget::reject() {
	itemToEdit = nullptr;

	QDialog::reject();
}

void FolderItemPropertiesWidget::sl_ButtonBox_Clicked(QAbstractButton* button) {
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

void FolderItemPropertiesWidget::sl_ChooseIconButton_Clicked() {
	if (itemToEdit == nullptr) {
		WARNING("Null pointer recieved");
		reject();
	}

	if (!customIconsWidget) {
		customIconsWidget = new CustomIconsListWidget(this);
	}

	if (itemToEdit->GetItemType() == AbstractFolderItem::Type_Folder) {
		Folder* f = dynamic_cast<Folder*>(itemToEdit);
		customIconsWidget->SelectIcon(f->GetIconID());
	} else if (itemToEdit->GetItemType() == AbstractFolderItem::Type_Note) {
		Note* n = dynamic_cast<Note*>(itemToEdit);
		customIconsWidget->SelectIcon(n->GetIconID());
	}

	if (customIconsWidget->exec() != QDialog::Accepted) {return;}

	Document* doc = Application::I()->CurrentDocument();
	if (doc == nullptr) {
		WARNING("Current document is null");
		return;
	}

	selectedIconKey = customIconsWidget->SelectedIconKey;
	iconLabel->setPixmap(doc->GetItemIcon(selectedIconKey));
}

void FolderItemPropertiesWidget::sl_ResetIconToDefaultButton_Clicked() {
	Document* doc = Application::I()->CurrentDocument();
	if (doc == nullptr) {
		WARNING("Current document is null");
		return;
	}

	if (itemToEdit->GetItemType() == AbstractFolderItem::Type_Folder) {
		iconLabel->setPixmap(doc->GetItemIcon(doc->GetDefaultFolderIcon()));
		selectedIconKey = doc->GetDefaultFolderIcon();
	} else if (itemToEdit->GetItemType() == AbstractFolderItem::Type_Note) {
		iconLabel->setPixmap(doc->GetItemIcon(doc->GetDefaultNoteIcon()));
		selectedIconKey = doc->GetDefaultNoteIcon();
	}
}

void FolderItemPropertiesWidget::sl_SetDefaultIconButton_Clicked() {
	Document* doc = Application::I()->CurrentDocument();
	if (doc == nullptr) {
		WARNING("Current document is null");
		return;
	}

	if (itemToEdit->GetItemType() == AbstractFolderItem::Type_Folder) {
		doc->SetDefaultFolderIcon(selectedIconKey);
	} else if (itemToEdit->GetItemType() == AbstractFolderItem::Type_Note) {
		doc->SetDefaultNoteIcon(selectedIconKey);
	}
}
