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

#include "foldernavigationwidget.h"

#include "hierarchymodel.h"
#include "application.h"
#include "document.h"
#include "abstractfolderitem.h"
#include "basemodelitem.h"
#include "foldermodelitem.h"
#include "notemodelitem.h"
#include "folder.h"
#include "note.h"
#include "folderitempropertieswidget.h"
#include "modelitemdelegate.h"
#include "global.h"
#include "custommessagebox.h"

#include <QVBoxLayout>
#include <QColorDialog>
#include <QKeyEvent>
#include <QRegExp>
#include <QStack>
#include <QCheckBox>
#include <QDebug>

using namespace qNotesManager;

FolderNavigationWidget::FolderNavigationWidget(QWidget *parent) : QWidget(parent) {
	treeView = new QTreeView();
	treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	treeView->setDragEnabled(true);
	treeView->setDropIndicatorShown(true);
	treeView->setDragDropMode(QAbstractItemView::InternalMove);
	treeView->viewport()->setAcceptDrops(true);
	treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	treeView->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);

	QObject::connect(treeView, SIGNAL(customContextMenuRequested(QPoint)),
					 this, SLOT(sl_TreeView_ContextMenuRequested(QPoint)));
	QObject::connect(treeView, SIGNAL(clicked(QModelIndex)),
					 this, SLOT(sl_View_clicked(QModelIndex)));
	QObject::connect(treeView, SIGNAL(doubleClicked(QModelIndex)),
					 this, SLOT(sl_View_doubleClicked(QModelIndex)));
	QObject::connect(treeView, SIGNAL(expanded(QModelIndex)),
					 this, SLOT(sl_View_Expanded(QModelIndex)));
	QObject::connect(treeView, SIGNAL(collapsed(QModelIndex)),
					 this, SLOT(sl_View_Collapsed(QModelIndex)));

	treeView->installEventFilter(this);
	treeView->setHeaderHidden(true);
	treeView->setEnabled(false);

	model = 0;

	currentRootLabel = new QLabel();
	currentRootLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
	currentRootLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);

	pinFolderButton = new QPushButton();
	pinFolderButton->setToolTip("Pin folder");
	pinFolderButton->setIcon(QIcon(":/gui/pin"));
	pinFolderButton->setCheckable(true);
	pinFolderButton->setChecked(false);
	pinFolderButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	pinFolderButton->setEnabled(false);;
	pinFolderButton->setFocusPolicy(Qt::NoFocus);
	QObject::connect(pinFolderButton, SIGNAL(toggled(bool)),
					 this, SLOT(sl_PinFolderButton_Toggled(bool)));


	QVBoxLayout* layout = new QVBoxLayout();
		QHBoxLayout* pinnedFolderLayout = new QHBoxLayout();
		pinnedFolderLayout->addWidget(currentRootLabel);
		pinnedFolderLayout->setStretchFactor(currentRootLabel, 0);
		pinnedFolderLayout->addWidget(pinFolderButton);
	layout->addLayout(pinnedFolderLayout);
	layout->addWidget(treeView);
	setLayout(layout);

	// Item actions
	folderContextMenu = new QMenu(this);
	noteContextMenu = new QMenu(this);
	defaultContextMenu = new QMenu(this);
	trashFolderMenu = new QMenu(this);

	separatorAction = new QAction(this);
	separatorAction->setSeparator(true);

	addTopLevelNote = new QAction("Add top-level note", this);
	QObject::connect(addTopLevelNote, SIGNAL(triggered()),
					 this, SLOT(sl_AddTopLevelNoteAction_Triggered()));

	addTopLevelFolder = new QAction("Add top-level folder", this);
	QObject::connect(addTopLevelFolder, SIGNAL(triggered()),
					 this, SLOT(sl_AddTopLevelFolderAction_Triggered()));

	addNoteAction = new QAction(QIcon(":/gui/doc-add"), "Add Note", this);
	QObject::connect(addNoteAction, SIGNAL(triggered()),
					 this, SLOT(sl_AddNoteAction_Triggered()));

	addFolderAction = new QAction (QIcon(":/gui/folder-add"), "Add Folder", this);
	QObject::connect(addFolderAction, SIGNAL(triggered()),
					 this, SLOT(sl_AddFolderAction_Triggered()));

	deleteItemAction = new QAction (QIcon(":/gui/bin"), "Delete", this);
	QObject::connect(deleteItemAction, SIGNAL(triggered()),
					 this, SLOT(sl_DeleteItemAction_Triggered()));

	itemPropertiesAction = new QAction(QIcon(":/gui/property"), "Properties", this);
	itemPropertiesAction->setShortcut(QKeySequence(Qt::Key_F4));
	itemPropertiesAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	QObject::connect(itemPropertiesAction, SIGNAL(triggered()),
					 this, SLOT(sl_PropertiesAction_Triggered()));

	lockItemAction = new QAction(QIcon(":/gui/lock"), "Lock", this);
	lockItemAction->setCheckable(true);
	lockItemAction->setChecked(false);
	QObject::connect(lockItemAction, SIGNAL(triggered(bool)),
					 this, SLOT(sl_LockItemAction_Triggered(bool)));


	itemForeColorMenu = new QMenu("Set text color", this);
	itemForeColorMenu->setIcon(QIcon(":/gui/text-color"));
	itemBackColorMenu = new QMenu("Set back color", this);
	itemBackColorMenu->setIcon(QIcon(":/gui/background-color"));
	itemDefaultForeColorAction = new QAction("Default color", this);
	QObject::connect(itemDefaultForeColorAction, SIGNAL(triggered()),
					 this, SLOT(sl_DefaultForeColor_Triggered()));

	itemCustomForeColorAction = new QAction("Custom color", this);
	QObject::connect(itemCustomForeColorAction, SIGNAL(triggered()),
					 this, SLOT(sl_CustomForeColor_Triggered()));

	itemDefaultBackColorAction = new QAction("Default color", this);
	QObject::connect(itemDefaultBackColorAction, SIGNAL(triggered()),
					 this, SLOT(sl_DefaultBackColor_Triggered()));

	itemCustomBackColorAction = new QAction("Custom color", this);
	QObject::connect(itemCustomBackColorAction, SIGNAL(triggered()),
					 this, SLOT(sl_CustomBackColor_Triggered()));

	clearTrashAction = new QAction("Clear trash", this);
	QObject::connect(clearTrashAction, SIGNAL(triggered()),
					 this, SLOT(sl_ClearTrashAction_Triggered()));

	openNoteAction = new QAction("Open", this);
	QObject::connect(openNoteAction, SIGNAL(triggered()),
					 this, SLOT(sl_OpenNoteAction_Triggered()));

	renameItemAction = new QAction(QIcon(":/gui/rename"), "Rename", this);
	QObject::connect(renameItemAction, SIGNAL(triggered()),
					 this, SLOT(sl_RenameItemAction_Triggered()));

	itemForeColorMenu->addAction(itemDefaultForeColorAction);
	itemForeColorMenu->addAction(itemCustomForeColorAction);
	itemBackColorMenu->addAction(itemDefaultBackColorAction);
	itemBackColorMenu->addAction(itemCustomBackColorAction);

	folderContextMenu->addAction(addNoteAction);
	folderContextMenu->addAction(addFolderAction);
	folderContextMenu->addAction(deleteItemAction);
	folderContextMenu->addAction(itemPropertiesAction);
	folderContextMenu->addAction(lockItemAction);
	folderContextMenu->addMenu(itemForeColorMenu);
	folderContextMenu->addMenu(itemBackColorMenu);

	noteContextMenu->addAction(deleteItemAction);
	noteContextMenu->addAction(itemPropertiesAction);
	noteContextMenu->addAction(lockItemAction);
	noteContextMenu->addMenu(itemForeColorMenu);
	noteContextMenu->addMenu(itemBackColorMenu);

	defaultContextMenu->addAction(addNoteAction);
	defaultContextMenu->addAction(addFolderAction);

	trashFolderMenu->addAction(clearTrashAction);

	propertiesWidget = new FolderItemPropertiesWidget(this);

	contextMenu = new QMenu(this);
}

void FolderNavigationWidget::SetCurrentItem(Note* note) {
	if (!note) {return;}

	model->sl_RequestEmitApplySelection(note);
}

void FolderNavigationWidget::sl_TreeView_ContextMenuRequested(const QPoint& p) {
	QList<QAction*> actions = GetSelectedItemsActions();
	contextMenu->clear();

	foreach (QAction* act, actions) {
		contextMenu->addAction(act);
	}

	contextMenu->exec(treeView->viewport()->mapToGlobal(p));
}

QList<QAction*> FolderNavigationWidget::GetSelectedItemsActions() const {
	QList<QAction*> list;

	const QItemSelectionModel* selectionModel = treeView->selectionModel();
	if (!selectionModel) {return list;}

	list.append(addTopLevelNote);
	list.append(addTopLevelFolder);
	list.append(separatorAction);

	const QModelIndexList indexesList = selectionModel->selectedIndexes();
	bool locked = false;

	// 0 items
	if (indexesList.isEmpty()) {
		if (model->GetPinnedFolder() != 0) {
			// Edd context menu items for pinned folder
			if (model->GetPinnedFolder() == Application::I()->CurrentDocument()->GetTrashFolder()) {
				list.append(clearTrashAction);
				if (Application::I()->CurrentDocument()->GetTrashFolder()->Items.Count() == 0) {
					clearTrashAction->setEnabled(false);
				} else {
					clearTrashAction->setEnabled(true);
				}
			} else if (model->GetPinnedFolder() == Application::I()->CurrentDocument()->GetTempFolder()) {
				list.append(addNoteAction);
				list.append(addFolderAction);
			} else {
				list.append(addNoteAction);
				list.append(addFolderAction);
				list.append(itemPropertiesAction);
			}

			locked = model->GetPinnedFolder()->IsLocked();

			addNoteAction->setEnabled(!locked);
			addFolderAction->setEnabled(!locked);
			itemPropertiesAction->setEnabled(true);
		}
		return list;

	} else if (indexesList.size() == 1) { // 1 item
		const QModelIndex index = indexesList.first();
		if (!index.isValid()) {
			return list;
		}

		const BaseModelItem* modelitem = static_cast<BaseModelItem*>(index.internalPointer());

		if (modelitem->DataType() == BaseModelItem::folder) {
			const FolderModelItem* folderModelItem = dynamic_cast<const FolderModelItem*>(modelitem);
			const AbstractFolderItem* folderItem = folderModelItem->GetStoredData();
			if (folderItem == Application::I()->CurrentDocument()->GetTrashFolder()) {
				list.append(clearTrashAction);
				if (Application::I()->CurrentDocument()->GetTrashFolder()->Items.Count() == 0) {
					clearTrashAction->setEnabled(false);
				} else {
					clearTrashAction->setEnabled(true);
				}
			} else if (folderItem == Application::I()->CurrentDocument()->GetTempFolder()) {
				list.append(addNoteAction);
				list.append(addFolderAction);
			} else {
				list.append(addNoteAction);
				list.append(addFolderAction);
				list.append(deleteItemAction);
				list.append(itemPropertiesAction);
				list.append(renameItemAction);
				list.append(lockItemAction);
				list.append(itemForeColorMenu->menuAction());
				list.append(itemBackColorMenu->menuAction());
			}

			locked = folderModelItem->GetStoredData()->IsLocked();

		} else if (modelitem->DataType() == BaseModelItem::note) {
			list.append(openNoteAction);
			list.append(deleteItemAction);
			list.append(itemPropertiesAction);
			list.append(renameItemAction);
			list.append(lockItemAction);
			list.append(itemForeColorMenu->menuAction());
			list.append(itemBackColorMenu->menuAction());
			locked = dynamic_cast<const NoteModelItem*>(modelitem)->GetStoredData()->IsLocked();
		} else {
			WARNING("Wrong item type");
		}

		lockItemAction->blockSignals(true);
		lockItemAction->setChecked(locked);
		lockItemAction->blockSignals(false);

		addNoteAction->setEnabled(!locked);
		addFolderAction->setEnabled(!locked);
		renameItemAction->setEnabled(!locked);
		deleteItemAction->setEnabled(true);
		itemPropertiesAction->setEnabled(true);
		itemForeColorMenu->setEnabled(!locked);
		itemBackColorMenu->setEnabled(!locked);

	} else { // few items
		list.append(deleteItemAction);
		list.append(itemPropertiesAction);
		list.append(lockItemAction);
		list.append(itemForeColorMenu->menuAction());
		list.append(itemBackColorMenu->menuAction());

		foreach (const QModelIndex index, indexesList) {
			const BaseModelItem* modelitem = static_cast<BaseModelItem*>(index.internalPointer());
			if (modelitem->DataType() == BaseModelItem::folder) {
				locked = dynamic_cast<const FolderModelItem*>(modelitem)->GetStoredData()->IsLocked();
			} else if (modelitem->DataType() == BaseModelItem::note) {
				locked = dynamic_cast<const NoteModelItem*>(modelitem)->GetStoredData()->IsLocked();
			}
			if (locked) break;
		}

		lockItemAction->blockSignals(true);
		lockItemAction->setChecked(locked);
		lockItemAction->blockSignals(false);

		itemPropertiesAction->setEnabled(false);

		itemForeColorMenu->setEnabled(!locked);
		itemBackColorMenu->setEnabled(!locked);
	}

	return list;
}

void FolderNavigationWidget::sl_PinFolderButton_Toggled(bool toggle) {
	if (!model || !Application::I()->CurrentDocument()) {
		currentRootLabel->setText("");
		currentRootLabel->setToolTip("");

		pinFolderButton->blockSignals(true);
		pinFolderButton->setChecked(false);
		pinFolderButton->blockSignals(false);
		return;
	}

	Folder* folderToPin = 0;
	if (toggle) {
		QModelIndex index = treeView->currentIndex();
		if (!index.isValid()) {
			WARNING("Invalid index");
			return;
		}

		BaseModelItem* modelItem = static_cast<BaseModelItem*>(index.internalPointer());
		if (modelItem->DataType() != BaseModelItem::folder) {
			pinFolderButton->blockSignals(true);
			pinFolderButton->setChecked(false);
			pinFolderButton->blockSignals(false);
			return;
		}

		folderToPin = dynamic_cast<FolderModelItem*>(modelItem)->GetStoredData();
	}

	Application::I()->CurrentDocument()->SetPinnedFolder(folderToPin);
	UpdatePinnedFolderData();
}

void FolderNavigationWidget::UpdatePinnedFolderData() {
	if (model &&
		model->GetPinnedFolder()) {
		QString path = model->GetPinnedFolder()->GetPath();
		currentRootLabel->setText(path);
		currentRootLabel->setToolTip(path);
	} else {
		currentRootLabel->setText("");
		currentRootLabel->setToolTip("");
	}
}

void FolderNavigationWidget::sl_AddTopLevelNoteAction_Triggered() {
	Application::I()->CurrentDocument()->GetRoot()->Items.Add(new Note());
}

void FolderNavigationWidget::sl_AddTopLevelFolderAction_Triggered() {
	Application::I()->CurrentDocument()->GetRoot()->Items.Add(new Folder());
}

void FolderNavigationWidget::sl_AddNoteAction_Triggered() {
	Document* doc = Application::I()->CurrentDocument();

	QModelIndexList indexesList = treeView->selectionModel()->selectedIndexes();
	if (indexesList.size() > 1) {
		WARNING("Wrong item selection");
		return;
	}

	Folder* parentFolder = 0;

	if (indexesList.size() == 0) {
		parentFolder = (model->GetPinnedFolder() == 0 ? doc->GetRoot() : model->GetPinnedFolder());
	} else {
		BaseModelItem* modelitem =
				static_cast<BaseModelItem*>(indexesList.value(0).internalPointer());
		if (modelitem->DataType() != BaseModelItem::folder) {
			WARNING("Parent item is not a folder");
			return;
		}
		parentFolder = dynamic_cast<FolderModelItem*>(modelitem)->GetStoredData();
		if (parentFolder == doc->GetTrashFolder()) {
			WARNING("Cannot create notes in bin");
			return;
		}
		if (parentFolder->IsLocked()) {
			WARNING("Parent folder is locked");
			return;
		}
	}

	Note* n = new Note();
	parentFolder->Items.Add(n);
}

void FolderNavigationWidget::sl_AddFolderAction_Triggered() {
	Document* doc = Application::I()->CurrentDocument();

	QModelIndexList indexesList = treeView->selectionModel()->selectedIndexes();
	if (indexesList.size() > 1) {
		WARNING("Wrong item selection");
		return;
	}

	Folder* parentFolder = 0;

	if (indexesList.size() == 0) {
		parentFolder = (model->GetPinnedFolder() == 0 ? doc->GetRoot() : model->GetPinnedFolder());
	} else {
		BaseModelItem* modelitem =
				static_cast<BaseModelItem*>(indexesList.value(0).internalPointer());

		if (modelitem->DataType() != BaseModelItem::folder) {
			WARNING("Parent item is not a folder");
			return;
		}
		parentFolder = dynamic_cast<FolderModelItem*>(modelitem)->GetStoredData();
		if (parentFolder == doc->GetTrashFolder()) {
			WARNING("Cannot create new items in system folders");
			return;
		}
		if (parentFolder->IsLocked()) {
			WARNING("Parent folder is locked");
			return;
		}
	}

	Folder* newFolder = new Folder();
	parentFolder->Items.Add(newFolder);
}

void FolderNavigationWidget::sl_DeleteItemAction_Triggered() {
	QModelIndexList list = treeView->selectionModel()->selectedIndexes();
	deleteItems(list, !Application::I()->Settings.GetMoveItemsToBin());
}

void FolderNavigationWidget::sl_PropertiesAction_Triggered() {
	QModelIndexList indexesList = treeView->selectionModel()->selectedIndexes();

	if (indexesList.size() > 1) {
		WARNING("Wrong selection for this operation");
		return;
	}

	AbstractFolderItem* itemToEdit = 0;

	if (indexesList.size() == 0) {
		itemToEdit = (model->GetPinnedFolder() == 0 ? Application::I()->CurrentDocument()->GetRoot() : model->GetPinnedFolder());
	} else {
		BaseModelItem* modelItemToEdit = 0;

		QModelIndex itemIndex = indexesList.at(0);
		if (!itemIndex.isValid()) {
			WARNING("Got invalid index");
			return;
		}

		modelItemToEdit =
				static_cast<BaseModelItem*>(itemIndex.internalPointer());

		if (modelItemToEdit->DataType() == BaseModelItem::folder) {
			Folder* f = (dynamic_cast<FolderModelItem*>(modelItemToEdit))->GetStoredData();
			if (!f) {
				WARNING("Casting error");
				return;
			}

			if (f->GetType() != Folder::UserFolder) {
				WARNING("Cannot perform action on system folder");
				return;
			}

			itemToEdit = f;
		} else if (modelItemToEdit->DataType() == BaseModelItem::note) {
			Note* n = (dynamic_cast<NoteModelItem*>(modelItemToEdit))->GetStoredData();
			if (!n) {
				WARNING("Casting error");
				return;
			}

			itemToEdit = n;
		}
	}

	propertiesWidget->SetFolderItem(itemToEdit);
	propertiesWidget->exec();
}

void FolderNavigationWidget::sl_DefaultForeColor_Triggered() {
	QModelIndexList indexesList = treeView->selectionModel()->selectedIndexes();
	if (indexesList.size() == 0) {
		WARNING("Wrong selection for this operation");
		return;
	}

	for (int i = 0; i < indexesList.size(); ++i) {
		if (!indexesList.value(i).isValid()) {
			WARNING("Invalid index");
			continue;
		}

		BaseModelItem* modelItemToEdit =
				static_cast<BaseModelItem*>(indexesList.value(i).internalPointer());

		if (modelItemToEdit->DataType() == BaseModelItem::folder) {
			Folder* f = (dynamic_cast<FolderModelItem*>(modelItemToEdit))->GetStoredData();
			if (!f) {
				WARNING("Casting error");
				continue;
			}
			if (f->GetType() != Folder::UserFolder) {
				WARNING("Cannot perform action on system folder");
				return;
			}
			if (f->IsLocked()) {
				WARNING("Item is locked");
				continue;
			}
			f->SetNameForeColor(f->GetDefaultForeColor());
		} else if (modelItemToEdit->DataType() == BaseModelItem::note) {
			Note* n = (dynamic_cast<NoteModelItem*>(modelItemToEdit))->GetStoredData();
			if (!n) {
				WARNING("Casting error");
				continue;
			}
			if (n->IsLocked()) {
				WARNING("Item is locked");
				continue;
			}
			n->SetNameForeColor(n->GetDefaultForeColor());
		}
	}
}

void FolderNavigationWidget::sl_DefaultBackColor_Triggered() {
	QModelIndexList indexesList = treeView->selectionModel()->selectedIndexes();
	if (indexesList.size() == 0) {
		WARNING("Wrong selection for this operation");
		return;
	}

	for (int i = 0; i < indexesList.size(); ++i) {
		if (!indexesList.value(i).isValid()) {
			WARNING("Invalid index");
			continue;
		}
		BaseModelItem* modelItemToEdit =
				static_cast<BaseModelItem*>(indexesList.value(i).internalPointer());

		if (modelItemToEdit->DataType() == BaseModelItem::folder) {
			Folder* f = (dynamic_cast<FolderModelItem*>(modelItemToEdit))->GetStoredData();
			if (!f) {
				WARNING("Casting error");
				continue;
			}
			if (f->GetType() != Folder::UserFolder) {
				WARNING("Cannot perform action on system folder");
				return;
			}
			if (f->IsLocked()) {
				WARNING("Item is locked");
				continue;
			}
			f->SetNameBackColor(f->GetDefaultBackColor());
		} else if (modelItemToEdit->DataType() == BaseModelItem::note) {
			Note* n = (dynamic_cast<NoteModelItem*>(modelItemToEdit))->GetStoredData();
			if (!n) {
				WARNING("Casting error");
				continue;
			}
			if (n->IsLocked()) {
				WARNING("Item is locked");
				continue;
			}
			n->SetNameBackColor(n->GetDefaultBackColor());
		}
	}
}

void FolderNavigationWidget::sl_CustomForeColor_Triggered() {
	QModelIndexList indexesList = treeView->selectionModel()->selectedIndexes();
	if (indexesList.size() == 0) {
		WARNING("Wrong selection for this operation");
		return;
	}

	QColor oldColor = QColor();
	if (indexesList.size() == 1) {
		BaseModelItem* modelItemToEdit =
				static_cast<BaseModelItem*>(indexesList.value(0).internalPointer());

		if (modelItemToEdit->DataType() == BaseModelItem::folder) {
			Folder* f = (dynamic_cast<FolderModelItem*>(modelItemToEdit))->GetStoredData();
			if (f) {
				oldColor = f->GetNameForeColor();
			}

		} else if (modelItemToEdit->DataType() == BaseModelItem::note) {
			Note* n = (dynamic_cast<NoteModelItem*>(modelItemToEdit))->GetStoredData();
			if (n) {
				oldColor = n->GetNameForeColor();
			}

		}
	}

	QColor newColor = QColorDialog::getColor(oldColor);
	if (!newColor.isValid()) {return;}

	for (int i = 0; i < indexesList.size(); ++i) {
		if (!indexesList.value(i).isValid()) {
			WARNING("Invalid index");
			continue;
		}
		BaseModelItem* modelItemToEdit =
				static_cast<BaseModelItem*>(indexesList.value(i).internalPointer());

		if (modelItemToEdit->DataType() == BaseModelItem::folder) {
			Folder* f = (dynamic_cast<FolderModelItem*>(modelItemToEdit))->GetStoredData();
			if (!f) {
				WARNING("Casting error");
				continue;
			}
			if (f->GetType() != Folder::UserFolder) {
				WARNING("Cannot perform action on system folder");
				return;
			}
			if (f->IsLocked()) {
				WARNING("Item is locked");
				continue;
			}
			f->SetNameForeColor(newColor);
		} else if (modelItemToEdit->DataType() == BaseModelItem::note) {
			Note* n = (dynamic_cast<NoteModelItem*>(modelItemToEdit))->GetStoredData();
			if (!n) {
				WARNING("Casting error");
				continue;
			}
			if (n->IsLocked()) {
				WARNING("Item is locked");
				continue;
			}
			n->SetNameForeColor(newColor);
		}
	}
}

void FolderNavigationWidget::sl_CustomBackColor_Triggered() {
	QModelIndexList indexesList = treeView->selectionModel()->selectedIndexes();
	if (indexesList.size() == 0) {
		WARNING("Wrong selection for this operation");
		return;
	}

	QColor oldColor = QColor();
	if (indexesList.size() == 1) {
		BaseModelItem* modelItemToEdit =
				static_cast<BaseModelItem*>(indexesList.value(0).internalPointer());

		if (modelItemToEdit->DataType() == BaseModelItem::folder) {
			Folder* f = (dynamic_cast<FolderModelItem*>(modelItemToEdit))->GetStoredData();
			if (f) {
				oldColor = f->GetNameBackColor();
			}

		} else if (modelItemToEdit->DataType() == BaseModelItem::note) {
			Note* n = (dynamic_cast<NoteModelItem*>(modelItemToEdit))->GetStoredData();
			if (n) {
				oldColor = n->GetNameBackColor();
			}
		}
	}

	QColor newColor = QColorDialog::getColor(oldColor);
	if (!newColor.isValid()) {return;}

	for (int i = 0; i < indexesList.size(); ++i) {
		if (!indexesList.value(i).isValid()) {
			WARNING("Invalid index");
			continue;
		}
		BaseModelItem* modelItemToEdit =
				static_cast<BaseModelItem*>(indexesList.value(i).internalPointer());

		if (modelItemToEdit->DataType() == BaseModelItem::folder) {
			Folder* f = (dynamic_cast<FolderModelItem*>(modelItemToEdit))->GetStoredData();
			if (!f) {
				WARNING("Casting error");
				continue;
			}
			if (f->GetType() != Folder::UserFolder) {
				WARNING("Cannot perform action on system folder");
				return;
			}
			if (f->IsLocked()) {
				WARNING("Item is locked");
				continue;
			}
			f->SetNameBackColor(newColor);
		} else if (modelItemToEdit->DataType() == BaseModelItem::note) {
			Note* n = (dynamic_cast<NoteModelItem*>(modelItemToEdit))->GetStoredData();
			if (!n) {
				WARNING("Casting error");
				continue;
			}
			if (n->IsLocked()) {
				WARNING("Item is locked");
				continue;
			}
			n->SetNameBackColor(newColor);
		}
	}
}

void FolderNavigationWidget::sl_ClearTrashAction_Triggered() {
	Folder* f = Application::I()->CurrentDocument()->GetTrashFolder();
	while (f->Items.Count() > 0) {
		AbstractFolderItem* item = f->Items.ItemAt(0);
		f->Items.Remove(item);
		delete item;
	}
}

void FolderNavigationWidget::sl_OpenNoteAction_Triggered() {
	QModelIndexList indexesList = treeView->selectionModel()->selectedIndexes();

	if (indexesList.size() == 0) {
		WARNING("Empty list");
		return;
	}

	foreach (const QModelIndex index, indexesList) {
		sl_View_doubleClicked(index);
	}
}

void FolderNavigationWidget::sl_RenameItemAction_Triggered() {
	if (!treeView->currentIndex().isValid()) {
		WARNING("Invalid index");
		return;
	}
	treeView->edit(treeView->currentIndex());
}

/* virtual */
bool FolderNavigationWidget::eventFilter (QObject* watched, QEvent* event) {
	if (watched != treeView) {return false;}
	if (event->type() != QEvent::KeyPress) {return false;}

	ModelItemDelegate* delegate = dynamic_cast<ModelItemDelegate*>(treeView->itemDelegate());
	if (delegate != 0 && delegate->isEditing()) {
		return false;
	}

	QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
	if (keyEvent->key() == Qt::Key_Delete) {
		// Delete selected items by pressing 'Delete' button
		if (treeView->selectionModel()->selectedIndexes().size() == 0) {return false;}
		bool permanently = false;
		if ((keyEvent->modifiers() & Qt::ShiftModifier)
			||
			!Application::I()->Settings.GetMoveItemsToBin()) {
			permanently = true;
		}
		QModelIndexList list = treeView->selectionModel()->selectedIndexes();
		deleteItems(list, permanently);

	} else if (keyEvent->key() == Qt::Key_Enter ||
			   keyEvent->key() == Qt::Key_Return) {
		// Open selected notes by pressing 'Enter' button
		if (treeView->selectionModel()->selectedIndexes().size() == 0) {return false;}
		QModelIndexList list = treeView->selectionModel()->selectedIndexes();

		foreach (QModelIndex index, list) {
			if (!index.isValid()) {continue;}

			BaseModelItem* item = static_cast<BaseModelItem*>(index.internalPointer());
			if (item->DataType() == BaseModelItem::note) {
				Note* note = dynamic_cast<NoteModelItem*>(item)->GetStoredData();
				if (!note) {
					WARNING("Casting error");
					continue;
				}
				emit sg_NoteDoubleClicked(note);
			}
		}
	} else if (keyEvent->key() == Qt::Key_F4) {
		sl_PropertiesAction_Triggered();
	}
	return false;
}

void FolderNavigationWidget::deleteItems(QModelIndexList& indexesList, bool permanently) {
	QString details;
	if (indexesList.size() == 0) {
		WARNING("List is empty");
		return;
	}

	foreach (QModelIndex index, indexesList) {
		details.append("\n").append(index.model()->data(index, Qt::DisplayRole).toString());
	}

	if (Application::I()->Settings.GetConfirmItemDeletion() == true) {
		QCheckBox* checkBox = new QCheckBox("Do not show this message again");
		QString message = permanently ? "Delete these items?" : "Put these items to Bin?";
		CustomMessageBox box(this, message + details, "Confirm deletion", QMessageBox::Question,
					   QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		box.AddCustomWidget(checkBox);
		QMessageBox::StandardButton result = box.show();

		if (result != QMessageBox::Yes) {
			return;
		}

		if (checkBox->isChecked()) {
			Application::I()->Settings.SetConfirmItemDeletion(false);
		}
	}

	// FIXME: fix situation when parent folder was deleted and we try to delete child item
	deleteChildIndexes(indexesList);

	foreach (QModelIndex index, indexesList) {
		if (!index.isValid()) {
			WARNING("Invalid index in list");
			continue;
		}

		BaseModelItem* modelItemToDelete = static_cast<BaseModelItem*>(index.internalPointer());

		AbstractFolderItem* itemToDelete = 0;
		if (modelItemToDelete->DataType() == BaseModelItem::folder) {
			FolderModelItem* fmi = dynamic_cast<FolderModelItem*>(modelItemToDelete);
			itemToDelete = fmi->GetStoredData();
			if (itemToDelete == Application::I()->CurrentDocument()->GetTempFolder() ||
				itemToDelete == Application::I()->CurrentDocument()->GetTrashFolder()) {
				CustomMessageBox msg(this, "You cannot delete system folders", "Information", QMessageBox::Information);
				msg.show();
				continue;
			}
		} else if (modelItemToDelete->DataType() == BaseModelItem::note) {
			NoteModelItem* fmi = dynamic_cast<NoteModelItem*>(modelItemToDelete);
			itemToDelete = fmi->GetStoredData();
		}
		if (itemToDelete == 0) {
			WARNING("Could not find an item for deletion");
			continue;
		}

		Folder* parentFolder = itemToDelete->GetParent();
		if (parentFolder == 0) {
			WARNING("Cannot delete item without parent");
			continue;
		}

		if (permanently) {
			parentFolder->Items.Remove(itemToDelete);
			delete itemToDelete;
		} else {
			parentFolder->Items.Move(itemToDelete, Application::I()->CurrentDocument()->GetTrashFolder());
		}
	}
}

void FolderNavigationWidget::deleteChildIndexes(QModelIndexList& list) const {
	QMutableListIterator<QModelIndex> iterator(list);

	while(iterator.hasNext()) {
		QModelIndex index = iterator.next();

		bool deleteCurrentIndex = false;
		QModelIndex parent = index.parent();
		while(parent.isValid()) {
			if (list.contains(parent)) {
				deleteCurrentIndex = true;
				break;
			}
			parent = parent.parent();
		}
		if (deleteCurrentIndex) {
			iterator.remove();
		}
	}
}

void FolderNavigationWidget::sl_Model_ApplySelection(const QModelIndexList& list) {
	treeView->selectionModel()->clearSelection();

	if (list.size() == 0) {return;}

	treeView->selectionModel()->setCurrentIndex(list.at(0), QItemSelectionModel::NoUpdate);

	foreach(QModelIndex index, list) {
		QModelIndex parentIndex = index.parent();
		while (parentIndex.isValid()) {
			treeView->expand(parentIndex);
			parentIndex = parentIndex.parent();
		}
		treeView->selectionModel()->select(index, QItemSelectionModel::Select);
	}
}

void FolderNavigationWidget::sl_LockItemAction_Triggered(bool checked) {
	QModelIndexList indexesList = treeView->selectionModel()->selectedIndexes();

	if (indexesList.size() == 0) {
		WARNING("Empty list");
		return;
	}

	Document* doc = Application::I()->CurrentDocument();

	for (int i = 0; i < indexesList.size(); ++i) {
		if (!indexesList.value(i).isValid()) {
			WARNING("Invalid item index");
			return;
		}
		BaseModelItem* modelItem = static_cast<BaseModelItem*>(indexesList.value(i).internalPointer());
		BaseModelItem* parentItem = modelItem->parent();

		// Do not allow to lock system folders
		if (modelItem->DataType() == BaseModelItem::folder) {
			const Folder* folder = dynamic_cast<FolderModelItem*>(modelItem)->GetStoredData();
			if (folder == doc->GetRoot() || folder == doc->GetTempFolder() ||
				folder == doc->GetTrashFolder()) {
				continue;
			}
		}

		// Do not allow to unlock child if Document::LockFolderItems is true and parent folder is locked
		if (doc->LockFolderItems &&
			dynamic_cast<FolderModelItem*>(parentItem)->GetStoredData()->IsLocked()) {
			continue;
		}

		if (modelItem->DataType() == BaseModelItem::folder) {
			dynamic_cast<FolderModelItem*>(modelItem)->GetStoredData()->SetLocked(checked);
		} else if (modelItem->DataType() == BaseModelItem::note) {
			dynamic_cast<NoteModelItem*>(modelItem)->GetStoredData()->SetLocked(checked);
		}
	}

	addNoteAction->setEnabled(!checked); // BAD!
	addFolderAction->setEnabled(!checked);
	itemForeColorMenu->setEnabled(!checked);
	itemBackColorMenu->setEnabled(!checked);
}

void FolderNavigationWidget::sl_View_clicked (const QModelIndex& index) {
	if (!index.isValid()) {return;}

	BaseModelItem* item = static_cast<BaseModelItem*>(index.internalPointer());
	if (item->DataType() == BaseModelItem::note) {
		Note* n = dynamic_cast<NoteModelItem*>(item)->GetStoredData();
		if (!n) {
			WARNING("Casting error");
			return;
		}
		emit sg_NoteClicked(n);
	}
}

void FolderNavigationWidget::sl_View_doubleClicked (const QModelIndex& index) {
	if (!index.isValid()) {return;}

	BaseModelItem* item = static_cast<BaseModelItem*>(index.internalPointer());
	if (item->DataType() == BaseModelItem::note) {
		Note* n = dynamic_cast<NoteModelItem*>(item)->GetStoredData();
		if (!n) {
			WARNING("Casting error");
			return;
		}
		emit sg_NoteDoubleClicked(n);
	}
}

void FolderNavigationWidget::sl_View_SelectionChanged(const QItemSelection&, const QItemSelection&) {
	emit sg_SelectedItemsActionsListChanged();
}

void FolderNavigationWidget::sl_View_Expanded(const QModelIndex& index) {
	if (!index.isValid()) {return;}
	if (index.internalPointer() == 0) {return;}

	BaseModelItem* modelItemToEdit =
			static_cast<BaseModelItem*>(index.internalPointer());

	if (modelItemToEdit->DataType() == BaseModelItem::folder) {
		Folder* folder = (dynamic_cast<FolderModelItem*>(modelItemToEdit))->GetStoredData();
		folder->SetExpanded(true);
	}
}

void FolderNavigationWidget::sl_View_Collapsed(const QModelIndex& index) {
	if (!index.isValid()) {return;}
	if (index.internalPointer() == 0) {return;}

	BaseModelItem* modelItemToEdit =
			static_cast<BaseModelItem*>(index.internalPointer());

	if (modelItemToEdit->DataType() == BaseModelItem::folder) {
		Folder* folder = (dynamic_cast<FolderModelItem*>(modelItemToEdit))->GetStoredData();
		folder->SetExpanded(false);
	}
}

void FolderNavigationWidget::restoreExpandedIndexes() {
	QStack<QModelIndex> indexes;
	indexes.push(treeView->rootIndex());

	while (!indexes.isEmpty()) {
		QModelIndex index = indexes.pop();
		for (int i = 0; i < model->rowCount(index); i++) {
			QModelIndex child = model->index(i, 0, index);
			if (child.isValid()) {
				indexes.push(child);
			}
		}

		if (!index.isValid()) {continue;}
		if (index.internalPointer() == 0) {continue;}
		BaseModelItem* modelItemToEdit =
				static_cast<BaseModelItem*>(index.internalPointer());

		if (modelItemToEdit->DataType() == BaseModelItem::folder) {
			Folder* folder = (dynamic_cast<FolderModelItem*>(modelItemToEdit))->GetStoredData();
			if (folder->IsExpanded()) {
				treeView->expand(index);
			}
		}
	}
}

void FolderNavigationWidget::sl_Model_DisplayRootItemChanged() {
	restoreExpandedIndexes();
	emit sg_SelectedItemsActionsListChanged();
}

void FolderNavigationWidget::SetModel(HierarchyModel* _model) {
	if (model) {
		QObject::disconnect(model, 0, this, 0);
	}

	model = _model;

	if (model) {
		QObject::connect(model, SIGNAL(sg_ApplySelection(QModelIndexList)),
						 this, SLOT(sl_Model_ApplySelection(QModelIndexList)));
		QObject::connect(model, SIGNAL(sg_DisplayRootItemChanged()),
						 this, SLOT(sl_Model_DisplayRootItemChanged()));
	}

	treeView->setModel(model);
	if (treeView->model() != 0) {
		for (int i = 0; i < treeView->model()->columnCount(); i++) {
			treeView->resizeColumnToContents(i);
		}
		QObject::connect(treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
						 this, SLOT(sl_View_SelectionChanged(QItemSelection,QItemSelection)));
		restoreExpandedIndexes();
	}

	treeView->setEnabled(model != 0);
	pinFolderButton->setEnabled(model != 0);

	if (model) {
		pinFolderButton->blockSignals(true);
		pinFolderButton->setChecked(model->GetPinnedFolder() != 0);
		pinFolderButton->blockSignals(false);
		UpdatePinnedFolderData();
	} else {
		pinFolderButton->blockSignals(true);
		pinFolderButton->setChecked(false);
		pinFolderButton->blockSignals(false);
		UpdatePinnedFolderData();
	}
}

void FolderNavigationWidget::SetModelItemDelegate(QItemDelegate* delegate) {
	treeView->setItemDelegate(delegate);
}
