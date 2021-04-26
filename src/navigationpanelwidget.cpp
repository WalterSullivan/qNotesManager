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

#include "navigationpanelwidget.h"

#include "foldernavigationwidget.h"
#include "tagsnavigationwidget.h"
#include "datenavigationwidget.h"
#include "document.h"
#include "application.h"
#include "modelitemdelegate.h"

#include <QVBoxLayout>

using namespace qNotesManager;

NavigationPanelWidget::NavigationPanelWidget(QWidget *parent) : QWidget(parent) {
	tabWidget = new QTabWidget();
	QObject::connect(tabWidget, SIGNAL(currentChanged(int)),
					 this, SLOT(sl_TabWidget_CurrentChanged(int)));

	QAbstractItemDelegate* delegate = new ModelItemDelegate(this);

	hierarchyWidget = new FolderNavigationWidget();
	hierarchyWidget->SetModelItemDelegate(delegate);
	QObject::connect(hierarchyWidget, SIGNAL(sg_NoteClicked(Note*)),
					 this, SIGNAL(sg_NoteClicked(Note*)));
	QObject::connect(hierarchyWidget, SIGNAL(sg_NoteDoubleClicked(Note*)),
					 this, SIGNAL(sg_NoteDoubleClicked(Note*)));
	QObject::connect(hierarchyWidget, SIGNAL(sg_SelectedItemsActionsListChanged()),
					 this, SIGNAL(sg_SelectedItemsActionsListChanged()));
	tabWidget->addTab(hierarchyWidget, QIcon(QPixmap(":/gui/folder-tree")), "Folders");

	tagsWidget = new TagsNavigationWidget();
	tagsWidget->SetModelItemDelegate(delegate);
	QObject::connect(tagsWidget, SIGNAL(sg_NoteClicked(Note*)),
					 this, SIGNAL(sg_NoteClicked(Note*)));
	QObject::connect(tagsWidget, SIGNAL(sg_NoteDoubleClicked(Note*)),
					 this, SIGNAL(sg_NoteDoubleClicked(Note*)));
	tabWidget->addTab(tagsWidget, QIcon(":/gui/tag"), "Tags"); // TODO: make icon and title widget's properties

	datesWidget = new DateNavigationWidget();
	datesWidget->SetModelItemDelegate(delegate);
	QObject::connect(datesWidget, SIGNAL(sg_NoteClicked(Note*)),
					 this, SIGNAL(sg_NoteClicked(Note*)));
	QObject::connect(datesWidget, SIGNAL(sg_NoteDoubleClicked(Note*)),
					 this, SIGNAL(sg_NoteDoubleClicked(Note*)));
	tabWidget->addTab(datesWidget, QIcon(":/gui/date"), "Dates");

	QVBoxLayout* layout = new QVBoxLayout();
#if QT_VERSION < 0x040300
	layout->setMargin(0);
#else
	layout->setContentsMargins(0, 0, 0, 0);
#endif
	layout->addWidget(tabWidget);
	setLayout(layout);

	UpdateViewsVisibility();
}

int NavigationPanelWidget::CurrentTabIndex() const {
	return tabWidget->currentIndex();
}

void NavigationPanelWidget::SetCurrentTab(int index) {
	if (index < 0 || index > tabWidget->count()) {return;}

	tabWidget->setCurrentIndex(index);
}

void NavigationPanelWidget::sl_SelectNoteInTree(Note* note, bool activateNavigationTab) {
	if (activateNavigationTab) {
		tabWidget->setCurrentWidget(hierarchyWidget);
	}
	hierarchyWidget->SetCurrentItem(note);
}

void NavigationPanelWidget::sl_TabWidget_CurrentChanged(int) {
	emit sg_SelectedItemsActionsListChanged();
}

void NavigationPanelWidget::SetTargetDocument(Document* document) {
	if (document) {
		hierarchyWidget->SetModel(document->GetHierarchyModel());
		datesWidget->SetCreationModel(document->GetCreationDatesModel());
		datesWidget->SetModificationModel(document->GetModificationDatesModel());
		datesWidget->SetTextDateModel(document->GetTextDatesModel());
		tagsWidget->SetModel(document->GetTagsModel());
	} else {
		hierarchyWidget->SetModel(0);
		datesWidget->SetCreationModel(0);
		datesWidget->SetModificationModel(0);
		datesWidget->SetTextDateModel(0);
		tagsWidget->SetModel(0);
	}
}

QList<QAction*> NavigationPanelWidget::GetSelectedItemsActions() const {
	if (tabWidget->currentWidget() == hierarchyWidget) {
		return  hierarchyWidget->GetSelectedItemsActions();
	}

	return QList<QAction*>();
}

void NavigationPanelWidget::UpdateViewsVisibility() {
	int tagsWidgetIndex = tabWidget->indexOf(tagsWidget);
	if (tagsWidgetIndex >= 0 && !Application::I()->Settings.GetShowTagsTreeView()) {
		tabWidget->removeTab(tagsWidgetIndex);
	} else if (tagsWidgetIndex == -1 && Application::I()->Settings.GetShowTagsTreeView()) {
		tabWidget->insertTab(1, tagsWidget, QIcon(":/gui/tag"), "Tags");
	}

	int datesWidgetIndex = tabWidget->indexOf(datesWidget);
	if (datesWidgetIndex >=0 && !Application::I()->Settings.GetShowDatesTreeView()) {
		tabWidget->removeTab(datesWidgetIndex);
	} else if (datesWidgetIndex == -1 && Application::I()->Settings.GetShowDatesTreeView()) {
		tabWidget->insertTab(2, datesWidget, QIcon(":/gui/date"), "Dates");
	}
}
