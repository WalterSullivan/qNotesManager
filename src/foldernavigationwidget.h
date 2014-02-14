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

#ifndef FOLDERNAVIGATIONWIDGET_H
#define FOLDERNAVIGATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTreeView>
#include <QListView>
#include <QComboBox>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QSortFilterProxyModel>
#include <QLineEdit>

namespace qNotesManager {
	class HierarchyModel;
	class FolderItemPropertiesWidget;
	class Note;
	class Folder;

	class FolderNavigationWidget : public QWidget {
	Q_OBJECT
	private:
		QTreeView*		treeView;
		HierarchyModel*	model;

		QLabel*			currentRootLabel;
		QPushButton*	pinFolderButton;

		QMenu*			folderContextMenu;
		QMenu*			noteContextMenu;
		QMenu*			defaultContextMenu;
		QMenu*			trashFolderMenu;
		QMenu*			itemForeColorMenu;
		QMenu*			itemBackColorMenu;

		QMenu*			contextMenu;

		QAction*		addTopLevelNote;
		QAction*		addTopLevelFolder;
		QAction*		addNoteAction;
		QAction*		addFolderAction;
		QAction*		deleteItemAction;
		QAction*		itemPropertiesAction;
		QAction*		lockItemAction;
		QAction*		itemDefaultForeColorAction;
		QAction*		itemCustomForeColorAction;
		QAction*		itemDefaultBackColorAction;
		QAction*		itemCustomBackColorAction;
		QAction*		clearTrashAction;
		QAction*		separatorAction;
		QAction*		openNoteAction;
		QAction*		renameItemAction;

		FolderItemPropertiesWidget* propertiesWidget;

		void deleteItems(QModelIndexList& list, bool permanently = false);
		// Delete indexes, which parents also in the list
		void deleteChildIndexes(QModelIndexList& list) const;

	public:
		explicit FolderNavigationWidget(QWidget *parent = 0);
		void SetCurrentItem(Note*);
		void SetModel(HierarchyModel* model);
		void SetPinnedFolder(Folder*);

		QList<QAction*> GetSelectedItemsActions() const;


		/*virtual*/
		bool eventFilter (QObject* watched, QEvent* event);

	signals:
		void sg_NoteClicked(Note*);
		void sg_NoteDoubleClicked(Note*);
		void sg_SelectedItemsActionsListChanged();

	private slots:
		void sl_TreeView_ContextMenuRequested(const QPoint& p);
		void sl_View_clicked (const QModelIndex&);
		void sl_View_doubleClicked (const QModelIndex&);
		void sl_View_SelectionChanged(const QItemSelection&, const QItemSelection&);

		void sl_PinFolderButton_Toggled(bool);

		// Actions event handlers
		void sl_AddTopLevelNoteAction_Triggered();
		void sl_AddTopLevelFolderAction_Triggered();
		void sl_AddNoteAction_Triggered();
		void sl_AddFolderAction_Triggered();
		void sl_DeleteItemAction_Triggered();
		void sl_PropertiesAction_Triggered();
		void sl_LockItemAction_Triggered(bool);
		void sl_DefaultForeColor_Triggered();
		void sl_DefaultBackColor_Triggered();
		void sl_CustomForeColor_Triggered();
		void sl_CustomBackColor_Triggered();
		void sl_ClearTrashAction_Triggered();
		void sl_OpenNoteAction_Triggered();
		void sl_RenameItemAction_Triggered();

		void sl_Model_ApplySelection(const QModelIndexList&);
		void sl_Model_DisplayRootItemChanged();
	};
}

#endif // FOLDERNAVIGATIONWIDGET_H
