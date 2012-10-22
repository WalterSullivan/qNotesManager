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

#ifndef NOTEEDITWIDGET_H
#define NOTEEDITWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDateTimeEdit>
#include <QScrollArea>
#include <QCheckBox>


namespace qNotesManager {
	class TextEditWidget;
	class Note;
	class NoteFragment;
	class TagsLineEdit;

	class NoteEditWidget : public QWidget {
	Q_OBJECT
	private:
		TextEditWidget*		textEditWidget;

		// Properties widget's controls
		QWidget*			propertiesWidget;
		QPushButton*		openClosePropertiesButton;
		QLabel*				captionLabel;
		QLineEdit*			captionEdit;
		QLabel*				authorLabel;
		QLineEdit*			authorEdit;
		QLabel*				textCreationDateLabel;
		QCheckBox*			textCreationCheckbox;
		QDateTimeEdit*		textCreationDateEdit;
		QLabel*				sourceLabel;
		QLineEdit*			sourceEdit;
		QLabel*				tagsLabel;
		TagsLineEdit*		tagsEdit;
		QLabel*				commentLabel;
		QLineEdit*			commentEdit;
		QScrollArea*		scrollArea;

		bool propertiesPanelCollapsed;
		void collapsePropertiesPanel();
		void expandPropertiesPanel();

		Note* currentNote;

		void updateControls();

	public:
		explicit NoteEditWidget(Note* n = 0);
		Note* CurrentNote() const;
		void ScrollTo(int position);
		void ShowFragment(const NoteFragment& fragment);
		int CurrentPosition() const;
		QList<QAction*> EditActionsList() const;

	protected:
		// virtual
		bool eventFilter (QObject* watched, QEvent* event);

	private slots:
		void sl_OpenClosePropertiesButton_Clicked();
		void sl_TextCreationCheckbox_Toggled(bool);
		void sl_TextCreationDateTime_Changed(const QDateTime&);
		void sl_TagsEdit_CollectionChanged(QStringList);
		void sl_Note_DataChanged();

	};
}

#endif // NOTEEDITWIDGET_H
