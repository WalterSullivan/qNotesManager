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

#ifndef CUSTOMICONSLISTWIDGET_H
#define CUSTOMICONSLISTWIDGET_H

#include <QDialog>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QButtonGroup>
#include <QDialogButtonBox>

namespace qNotesManager {
	class Document;

	class CustomIconsListWidget : public QDialog {
	Q_OBJECT
	private:
		QButtonGroup*	buttonGroup;
		QListView*		listView;
		QDialogButtonBox* buttonBox;

		QAbstractItemModel* iconsModel;
		QSortFilterProxyModel* filterModel;

		const int IconIDRole = Qt::UserRole + 1;
		const int IconGroupRole = Qt::UserRole + 2;

		int FindButtonIndexByName(const QString& name) const;
		void addCustomIcon();

	public:
		explicit CustomIconsListWidget(QWidget *parent = nullptr);
		QString SelectedIconKey;

		void SelectIcon(const QString& key);

	signals:

	public slots:
		virtual void accept();
		virtual void reject();

	private slots:
		void sl_ListView_DoubleClicked (const QModelIndex& index);
		void sl_ButtonGroup_ButtonClicked(QAbstractButton* button);
		void sl_ButtonBox_Clicked(QAbstractButton* button);
	};
}

#endif // CUSTOMICONSLISTWIDGET_H
