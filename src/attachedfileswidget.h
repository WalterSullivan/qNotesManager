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

#ifndef ATTACHEDFILESWIDGET_H
#define ATTACHEDFILESWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>

namespace qNotesManager {
	class Note;

	class AttachedFilesWidget : public QWidget {
	Q_OBJECT
	private:
		QLabel* info;
		QPushButton* addButton;
		QPushButton* saveButton;
		QPushButton* deleteButton;
		QListWidget* listWidget;

		QVBoxLayout* layout;

		Note* currentNote;

		void updateData();
		void updateCaption();
		void updateListWidgetHeight();

	public:
		explicit AttachedFilesWidget(Note* note, QWidget *parent = nullptr);
		void SetFocusPolicyCustom(Qt::FocusPolicy);

	protected:
		virtual void resizeEvent (QResizeEvent* event);

	signals:
		void sg_OnResize();

	public slots:


	private slots:
		void sl_AddButton_Clicked();
		void sl_SaveButton_Clicked();
		void sl_DeleteButton_Clicked();

		void sl_ListWidget_SelectionChanged();

		void sl_Note_PropertiesChanged();

	};
}

#endif // ATTACHEDFILESWIDGET_H
