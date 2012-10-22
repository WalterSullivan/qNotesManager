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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>

#include "application.h"
#include "document.h"

using namespace qNotesManager;

CustomIconsListWidget::CustomIconsListWidget(QWidget *parent) : QDialog(parent) {
	listView = new QListView();
	listView->setViewMode(QListView::IconMode);
	listView->setDragEnabled(false);
	listView->setSelectionMode(QAbstractItemView::SingleSelection);
	listView->setResizeMode(QListView::Adjust);
	// myListWidget->setStyleSheet( "QListWidget::item { border-bottom: 1px solid black; }" );
	listView->setModel(Application::I()->CurrentDocument()->customIconsModel);
	QObject::connect(listView, SIGNAL(doubleClicked(QModelIndex)),
					 this, SLOT(sl_ListView_DoubleClicked(QModelIndex)));

	okButton = new QPushButton("Select");
	QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(sl_OKButton_Clicked()));

	cancelButton = new QPushButton("Cancel");
	QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(sl_CancelButton_Clicked()));

	addIconButton = new QPushButton("Add custom icon");
	QObject::connect(addIconButton, SIGNAL(clicked()), this, SLOT(sl_AddIconButton_Clicked()));

	QHBoxLayout* hl = new QHBoxLayout();
	hl->addWidget(addIconButton);
	hl->addStretch();
	hl->addWidget(okButton);
	hl->addWidget(cancelButton);

	QVBoxLayout* vl = new QVBoxLayout();
	vl->addWidget(listView);
	vl->addLayout(hl);

	setLayout(vl);

	setWindowTitle("Pick icon");

	SelectedIconKey = "";
	SelectedIcon = QPixmap();
}

void CustomIconsListWidget::sl_OKButton_Clicked() {
	QModelIndex index = listView->currentIndex();
	if (!index.isValid()) {reject();}

	SelectedIconKey = index.data(Qt::UserRole + 1).toString();
	Q_ASSERT(!SelectedIconKey.isNull() && !SelectedIconKey.isEmpty());
	SelectedIcon = Application::I()->CurrentDocument()->GetItemIcon(SelectedIconKey);
	Q_ASSERT(!SelectedIcon.isNull());
	accept();
}

void CustomIconsListWidget::sl_CancelButton_Clicked() {
	SelectedIconKey = "";
	SelectedIcon = QPixmap();
	reject();
}

void CustomIconsListWidget::sl_AddIconButton_Clicked() {
	QString filter = "Images (*.png)";
	QStringList list = QFileDialog::getOpenFileNames (0, "Select icons to add", QString(),
													  filter);

	foreach (QString fileName, list) {
		QFileInfo info(fileName);
		if (!info.exists()) {continue;}
		QPixmap image(fileName, info.suffix().toStdString().c_str());
		if (image.isNull()) {continue;}
		Application::I()->CurrentDocument()->AddCustomIcon(image, info.fileName());
	}
}

void CustomIconsListWidget::SelectIcon(QString key) {
	Q_ASSERT(!key.isEmpty());

	for (int i = 0; i < listView->model()->rowCount(); ++i) {
		QModelIndex index = listView->model()->index(i, 0);
		if (!index.isValid()) {return;}
		QString itemKey = index.data(Qt::UserRole + 1).toString();
		Q_ASSERT(!itemKey.isEmpty());
		if (itemKey == key) {
			listView->scrollTo(index, QAbstractItemView::PositionAtCenter);
			listView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
			break;
		}
	}
}

void CustomIconsListWidget::sl_ListView_DoubleClicked (const QModelIndex& index) {
	sl_OKButton_Clicked();
}
