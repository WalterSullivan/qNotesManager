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

#include "attachedfileswidget.h"

#include <QScrollArea>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QDebug>

#include "note.h"
#include "cachedfile.h"
#include "custommessagebox.h"

using namespace qNotesManager;

AttachedFilesWidget::AttachedFilesWidget(Note* note, QWidget *parent) : QWidget(parent) {
	currentNote = note;
	QObject::connect(note, SIGNAL(sg_PropertyChanged()),
					 this, SLOT(sl_Note_PropertiesChanged()));

	info = new QLabel();

	addButton = new QPushButton();
	addButton->setToolTip("Attach file");
	addButton->setIcon(QIcon(":/gui/plus"));
	addButton->setFlat(true);
	addButton->setFocusPolicy(Qt::NoFocus);
	QObject::connect(addButton, SIGNAL(clicked()), this, SLOT(sl_AddButton_Clicked()));

	saveButton = new QPushButton();
	saveButton->setToolTip("Save file to...");
	saveButton->setIcon(QIcon(":/gui/disk-black"));
	saveButton->setFlat(true);
	saveButton->setEnabled(false);
	saveButton->setFocusPolicy(Qt::NoFocus);
	QObject::connect(saveButton, SIGNAL(clicked()), this, SLOT(sl_SaveButton_Clicked()));

	deleteButton = new QPushButton();
	deleteButton->setToolTip("Delete attached file");
	deleteButton->setIcon(QIcon(":/gui/cross"));
	deleteButton->setFlat(true);
	deleteButton->setEnabled(false);
	deleteButton->setFocusPolicy(Qt::NoFocus);
	QObject::connect(deleteButton, SIGNAL(clicked()), this, SLOT(sl_DeleteButton_Clicked()));

	listWidget = new QListWidget();
	listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	QObject::connect(listWidget, SIGNAL(itemSelectionChanged()),
					 this, SLOT(sl_ListWidget_SelectionChanged()));
	listWidget->setFixedHeight(100);

	QHBoxLayout* hlayout = new QHBoxLayout();
	hlayout->addWidget(info);
	hlayout->addWidget(addButton);
	hlayout->addWidget(saveButton);
	hlayout->addWidget(deleteButton);
	hlayout->addStretch();


	// Setting up main layout
	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addLayout(hlayout);
	mainLayout->addWidget(listWidget);

	setLayout(mainLayout);

	updateData();
}

void AttachedFilesWidget::SetFocusPolicyCustom(Qt::FocusPolicy policy) {
	listWidget->setFocusPolicy(policy);
}

void AttachedFilesWidget::sl_AddButton_Clicked() {
	if (currentNote == nullptr) {return;}

	QString fileName = QString();

	while(fileName.isEmpty()) {
		fileName = QFileDialog::getOpenFileName(this, "Select a file to attach", QString(),
												QString());
		if (fileName.isNull()) {return;}

		QFile file(fileName);
		if (fileName.isEmpty() || !file.exists()) {
			CustomMessageBox msg(this, "Select file to open", "Warning", QMessageBox::Warning);
			msg.show();
		} else {
			break;
		}
	}

	CachedFile* newFile = CachedFile::FromFile(fileName);
	if (newFile == nullptr || newFile->Size() == 0) {
		CustomMessageBox msg(this, "Failed to open file", "Warning", QMessageBox::Warning);
		msg.show();
		delete newFile;
		return;
	}

	// Check if this file was already attached
	quint32 newFileCRC = newFile->GetCRC32();
	for (int i = 0; i < currentNote->GetAttachedFilesCount(); i++) {
		const CachedFile* file = currentNote->GetAttachedFile(i);
		quint32 fileCRC = file->GetCRC32();
		if (newFileCRC == fileCRC && newFile->HasSameDataAs(file)) {
			CustomMessageBox msg(this, "Selected file is already attached. Proceed anyway?", "Warning",
						   QMessageBox::Question, QMessageBox::Yes | QMessageBox::No);
			QMessageBox::StandardButton answer = msg.show();
			if (answer != QMessageBox::Yes) {
				delete newFile;
				return;
			} else {
				break;
			}
		}
	}

	currentNote->AttachFile(newFile);
	updateData();
}

void AttachedFilesWidget::sl_SaveButton_Clicked() {
	if (currentNote == nullptr) {return;}

	const QListWidgetItem* currentItem = listWidget->currentItem();
	if (currentItem == nullptr) {return;}

	const QVariant indexData = currentItem->data(Qt::UserRole);
	if (indexData.isNull() || !indexData.isValid()) {return;}
	const int fileIndex = indexData.toInt();

	const CachedFile* attachedFile = currentNote->GetAttachedFile(fileIndex);
	if (attachedFile == nullptr) {return;}

	QString fileName = QString();

	while(fileName.isEmpty()) {
		fileName = QFileDialog::getSaveFileName(this, "Save as", attachedFile->GetFileName(), "");
		if (fileName.isNull()) {return;}
	}

	bool result = attachedFile->Save(fileName);
	if (!result) {
		CustomMessageBox msg(this, "Error happened during saving file", "Error", QMessageBox::Warning);
		msg.show();
	}
}

void AttachedFilesWidget::sl_DeleteButton_Clicked() {
	if (currentNote == nullptr) {return;}

	QList<QListWidgetItem*> selectedItems = listWidget->selectedItems();
	if (selectedItems.size() == 0) {return;}

	CustomMessageBox msg(this, "Delete selected files?", "Warning", QMessageBox::Question,
				   QMessageBox::Yes | QMessageBox::No);
	QMessageBox::StandardButton answer = msg.show();

	if (answer != QMessageBox::Yes) {
		return;
	}

	QList<CachedFile*> filesToDelete;
	foreach (const QListWidgetItem* item, selectedItems) {
		const QVariant indexData = item->data(Qt::UserRole);
		if (indexData.isNull() || !indexData.isValid()) {return;}
		const int fileIndex = indexData.toInt();
		CachedFile* file = currentNote->GetAttachedFile(fileIndex);
		if (file == nullptr) {continue;}

		filesToDelete.push_back(file);
	}

	foreach (CachedFile* f, filesToDelete) {
		currentNote->RemoveAttachedFile(f);
		delete f;
	}
}

void AttachedFilesWidget::updateData() {
	listWidget->clear();
	QIcon icon = QIcon(":/gui/clip");


	/*
		QFileInfo info(fileName);
		QFileIconProvider ip;
		icon = ip.icon(info);
		*/

	if (currentNote != nullptr) {
		for (int i = 0; i < currentNote->GetAttachedFilesCount(); i++) {
			const CachedFile* file = currentNote->GetAttachedFile(i);
			if (file == nullptr) {continue;}

			QListWidgetItem* newItem = new QListWidgetItem(icon, file->GetFileName());
			newItem->setData(Qt::UserRole, i);
			listWidget->addItem(newItem);
		}
	}

	updateCaption();
	updateListWidgetHeight();
}

void AttachedFilesWidget::updateCaption() {
	int attachedFilesCount = 0;
	quint32 attachedFilesTotalSize = 0;
	QString attachedFilesTotalSizeStr = "0 Kb";

	if (currentNote != nullptr) {
		attachedFilesCount = currentNote->GetAttachedFilesCount();
		for (int i = 0; i < attachedFilesCount; i++) {
			const CachedFile* file = currentNote->GetAttachedFile(i);
			attachedFilesTotalSize += file->Size();
		}

		if(attachedFilesTotalSize > 1048576) {
			attachedFilesTotalSizeStr = QString("%1 Mb").arg(attachedFilesTotalSize / 1048576);
		} else if (attachedFilesTotalSize > 1024) {
			attachedFilesTotalSizeStr = QString("%1 Kb").arg(attachedFilesTotalSize / 1024);
		} else {
			attachedFilesTotalSizeStr = QString("%1 bytes").arg(attachedFilesTotalSize);
		}
	}

	QString defaultMessage = QString("Attach files. Currently attached: %1").arg(
			QString::number(attachedFilesCount));
	QString totalSizeMessage = QString(", total size: %1").arg(attachedFilesTotalSizeStr);

	if (attachedFilesCount > 0) {
		defaultMessage.append(totalSizeMessage);
	}

	info->setText(defaultMessage);
}

void AttachedFilesWidget::updateListWidgetHeight() {
	if (listWidget->count() == 0) {
		listWidget->setVisible(false);
		return;
	}

	listWidget->setVisible(true);
	listWidget->setFixedHeight(100);
}

void AttachedFilesWidget::resizeEvent (QResizeEvent* event) {
	QWidget::resizeEvent(event);
	emit sg_OnResize();
}

void AttachedFilesWidget::sl_ListWidget_SelectionChanged() {
	QList<QListWidgetItem*> selectedItems = listWidget->selectedItems();

	const bool enableButton = (selectedItems.count() != 0);

	saveButton->setEnabled(enableButton);
	deleteButton->setEnabled(enableButton);
}

void AttachedFilesWidget::sl_Note_PropertiesChanged() {
	updateData();
}
