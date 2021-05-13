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

#ifndef NOTESTABWIDGET_H
#define NOTESTABWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QHash>
#include <QMenu>
#include <QUrl>

#if QT_VERSION < 0x040500
	#include <QPushButton>
#endif

#include "notefragment.h"

namespace qNotesManager {
	class NoteEditWidget;
	class Note;


	class NotesTabWidget : public QWidget {
	Q_OBJECT

	private:
		QHash<const Note*, QWidget*> hash;
		QTabWidget*		tabWidget;

	#if QT_VERSION < 0x040500
		QPushButton* closeTabButton;
	#endif

		void closeTab(int);
		QString cropStringForTabCaption(QString) const;

		QAction* closeTabAction;

	public:
		explicit NotesTabWidget(QWidget *parent = nullptr);

		void OpenNote(Note* note, int position = 0, bool newTab = false);
		void CloseNote(const Note* n);
		void ShowFragment(const NoteFragment& fragment);
		void Clear();
		QList<QAction*> GetCurrentEditActionsList() const;

		QList<const Note*> DisplayedNotes() const;
		Note* CurrentNote() const;
		void SetCurrentNote(const Note*);
		QList< QPair<Note*, int> > GetState() const;

		void ForceWriteTags();

	signals:
		void sg_CurrentNoteChanged(Note*);
		void sg_LinkClicked(QUrl url);

	private slots:
		void sl_Note_PropertiesChanged();
		void sl_TabWidget_CurrentChanged(int);
	#if QT_VERSION < 0x040500
		void sl_CloseWidgetButton_Clicked();
	#else
		void sl_TabWidget_TabCloseRequested(int);
	#endif
		void sl_CloseCurrentTab();

	};
}

#endif // NOTESTABWIDGET_H
