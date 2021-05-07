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

#include "customiconslistwidget.h"

#include "application.h"
#include "document.h"
#include "global.h"
#include "cachedimagefile.h"
#include "iconitemdelegate.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>

using namespace qNotesManager;

CustomIconsListWidget::CustomIconsListWidget(QWidget *parent) : QDialog(parent) {
	iconsModel = Application::I()->GetIconsModel();

	filterModel = new QSortFilterProxyModel(this);
	filterModel->setSourceModel(iconsModel);
	filterModel->setFilterRole(IconGroupRole);

	buttonGroup = new QButtonGroup(this);
	QObject::connect(buttonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
					 this, SLOT(sl_ButtonGroup_ButtonClicked(QAbstractButton*)));

	QVBoxLayout* buttonsLayout = new QVBoxLayout();

	QPushButton* b = nullptr;
	foreach (QString groupName, Application::I()->GetStandardIconGroups()) {
		b = new QPushButton(groupName, this);
		b->setCheckable(true);
		b->setFlat(true);
		b->setFocusPolicy(Qt::NoFocus);

		buttonGroup->addButton(b);
		buttonsLayout->addWidget(b);
	}

	b = new QPushButton("Custom icons", this);
	b->setCheckable(true);
	b->setFlat(true);
	b->setFocusPolicy(Qt::NoFocus);

	buttonGroup->addButton(b);
	buttonsLayout->addWidget(b);
	buttonsLayout->addStretch();


	listView = new QListView();
	listView->setViewMode(QListView::IconMode);
	listView->setDragEnabled(false);
	listView->setSelectionMode(QAbstractItemView::SingleSelection);
	listView->setResizeMode(QListView::Adjust);
	listView->setUniformItemSizes(true);
	listView->setWrapping(true);
#if QT_VERSION >= 0x040700
	listView->setSpacing(10); // There is a bug with setSpacing, fixed in 4.7
#else
	listView->setGridSize(QSize(30, 30));
#endif
	listView->setModel(filterModel);

	QStyledItemDelegate* delegate = new IconItemDelegate(this);
	listView->setItemDelegate(delegate);


	QObject::connect(listView, SIGNAL(doubleClicked(QModelIndex)),
					 this, SLOT(sl_ListView_DoubleClicked(QModelIndex)));

	okButton = new QPushButton("Select");
	QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

	cancelButton = new QPushButton("Cancel");
	QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	QObject::connect(this, SIGNAL(rejected()), this, SLOT(sl_Rejected()));

	addIconButton = new QPushButton("Add custom icon");
	QObject::connect(addIconButton, SIGNAL(clicked()), this, SLOT(sl_AddIconButton_Clicked()));

	QHBoxLayout* hl = new QHBoxLayout();
	hl->addWidget(addIconButton);
	hl->addStretch();
	hl->addWidget(okButton);
	hl->addWidget(cancelButton);

	QHBoxLayout* hl2 = new QHBoxLayout();
	hl2->addLayout(buttonsLayout);
	hl2->addWidget(listView);

	QVBoxLayout* vl = new QVBoxLayout();
	vl->addLayout(hl2);
	vl->addLayout(hl);

	setLayout(vl);

	setWindowTitle("Pick icon");
	resize(500, 500);

	SelectedIconKey = "";
}

void CustomIconsListWidget::accept() {
	QModelIndex index = listView->currentIndex();
	if (!index.isValid()) {return;}

	SelectedIconKey = index.data(IconIDRole).toString();
	if (SelectedIconKey.isEmpty()) {
		WARNING("Selected icon key is empty");
		return;
	}

	QDialog::accept();
}

void CustomIconsListWidget::sl_Rejected() {
	SelectedIconKey = "";
}

void CustomIconsListWidget::sl_AddIconButton_Clicked() {
	QString filter = "Images (*.png)";
	QStringList list = QFileDialog::getOpenFileNames (this, "Select icons to add", QString(),
													  filter);
	if (list.isEmpty()) {return;}

	foreach (QString fileName, list) {
		QFileInfo info(fileName);
		if (!info.exists()) {continue;}

		CachedImageFile* image = CachedImageFile::FromFile(fileName);
		if (!image->IsValidImage()) {
			WARNING("Selected file is not a valid image");
			delete image;
			continue;
		}

		Application::I()->CurrentDocument()->AddCustomIcon(image);
	}

	int newIndex = FindButtonIndexByName("Custom icons");
	if (newIndex == -1) {
		return;
	}
	QAbstractButton* b = buttonGroup->button(newIndex);
	b->click();
}

void CustomIconsListWidget::SelectIcon(const QString& key) {
	if (key.isEmpty()) {
		WARNING("Selected icon key is empty");
		return;
	}

	QModelIndex start = iconsModel->index(0, 0);

	QModelIndexList list = iconsModel->match(start, IconIDRole, key,
												1, Qt::MatchFixedString);

	if (list.isEmpty()) {
		WARNING("NO ICON FOUND");
		return;
	}

	QModelIndex index = list[0];
	QString group = index.data(IconGroupRole).toString();
	int newIndex = FindButtonIndexByName(group);

	if (newIndex == -1) {
		WARNING("Button not found");
		return;
	}

	QAbstractButton* b = buttonGroup->button(newIndex);
	b->click();

	index = filterModel->mapFromSource(index);

	if (!index.isValid()) {
		WARNING("Mapped index is invalid");
		return;
	}

	listView->scrollTo(index, QAbstractItemView::PositionAtCenter);
	listView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
}

void CustomIconsListWidget::sl_ListView_DoubleClicked (const QModelIndex& index) {
	(void)index;
	accept();
}

void CustomIconsListWidget::sl_ButtonGroup_ButtonClicked(QAbstractButton*) {
	QString group = buttonGroup->checkedButton()->text();
	filterModel->setFilterRegExp(QRegExp(group, Qt::CaseInsensitive, QRegExp::FixedString));
}

int CustomIconsListWidget::FindButtonIndexByName(const QString& name) const {
	QList<QAbstractButton*> buttons = buttonGroup->buttons();
	for (int i = 0; i < buttons.count(); i++) {
		if (buttons.at(i)->text() == name) {
			return buttonGroup->id(buttons.at(i));
		}
	}
	return -1;
}
