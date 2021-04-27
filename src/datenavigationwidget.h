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

#ifndef DATENAVIGATIONWIDGET_H
#define DATENAVIGATIONWIDGET_H

#include <QWidget>
#include <QTreeView>
#include <QGroupBox>
#include <QRadioButton>

namespace qNotesManager {
	class Note;
	class DatesModel;

	class DateNavigationWidget : public QWidget {
	Q_OBJECT
	public:
		explicit DateNavigationWidget(QWidget *parent = nullptr);

		void SetCreationModel(DatesModel*);
		void SetModificationModel(DatesModel*);
		void SetTextDateModel(DatesModel*);

		void SetModelItemDelegate(QAbstractItemDelegate*);

	private:
		QGroupBox*		groupBox;
		QRadioButton*	creationDateButton;
		QRadioButton*	modificationDateButton;
		QRadioButton*	textDateButton;
		QTreeView*		treeView;
		DatesModel*		creationDateModel;
		DatesModel*		modificationDateModel;
		DatesModel*		textDateModel;



	signals:
		void sg_NoteClicked(Note*);
		void sg_NoteDoubleClicked(Note*);

	private slots:
		void sl_View_clicked (const QModelIndex&);
		void sl_View_doubleClicked (const QModelIndex&);

		void sl_updateTreeModel(bool = false);

	};
}

#endif // DATENAVIGATIONWIDGET_H
