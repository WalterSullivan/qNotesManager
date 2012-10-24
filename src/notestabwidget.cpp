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

#include "notestabwidget.h"
#include "noteeditwidget.h"
#include "note.h"

#include <QBoxLayout>

using namespace qNotesManager;

NotesTabWidget::NotesTabWidget(QWidget *parent) : QWidget(parent) {
	tabWidget = new QTabWidget(this);
	QObject::connect(tabWidget, SIGNAL(currentChanged(int)),
					 this, SLOT(sl_TabWidget_CurrentChanged(int)));

#if QT_VERSION < 0x040500
	closeTabButton = new QPushButton();
	closeTabButton->setToolTip("Close tab");
	closeTabButton->setIcon(QIcon(":/gui/cross-small"));
	closeTabButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	QObject::connect(closeTabButton, SIGNAL(clicked()),
					 this, SLOT(sl_CloseWidgetButton_Clicked()));
	tabWidget->setCornerWidget(closeTabButton);
#else
	tabWidget->setTabsClosable(true);
	QObject::connect(tabWidget, SIGNAL(tabCloseRequested(int)),
					 this, SLOT(sl_TabWidget_TabCloseRequested(int)));
#endif


	layout = new QBoxLayout(QBoxLayout::LeftToRight);
#if QT_VERSION < 0x040300
	layout->setMargin(0);
#else
	layout->setContentsMargins(0, 0, 0, 0);
#endif

	layout->addWidget(tabWidget);

	setLayout(layout);
}

void NotesTabWidget::OpenNote(Note* note, int position, bool newTab) {
	(void)newTab;
	Q_ASSERT(note != 0);

	if (hash.contains(note)) {
		QWidget* noteWidget = hash.value(note);
		tabWidget->setCurrentWidget(noteWidget);
		return;
	}

	NoteEditWidget* w = new NoteEditWidget(note);
	hash.insert(note, w);
	QObject::connect(note, SIGNAL(sg_VisualPropertiesChanged()),
					 this, SLOT(sl_Note_PropertiesChanged()));


	tabWidget->addTab(w, note->GetIcon(), note->GetName());
	tabWidget->setCurrentWidget(w);
	w->ScrollTo(position);
}

void NotesTabWidget::CloseNote(const Note* n) {
	Q_ASSERT(n != 0);

	if (!hash.contains(n)) {return;}

	QObject::disconnect(n, 0, this, 0);

	QWidget* widget = hash.take(n);
	int tabIndex = tabWidget->indexOf(widget);
	Q_ASSERT(tabIndex != -1);
	tabWidget->removeTab(tabIndex);
	delete widget;
}

void NotesTabWidget::ShowFragment(const NoteFragment& fragment) {
	if (!hash.contains(fragment.NotePrt)) {
		OpenNote(const_cast<Note*>(fragment.NotePrt));
	}
	NoteEditWidget* noteWidget =
			dynamic_cast<NoteEditWidget*>(hash.value(fragment.NotePrt));
	noteWidget->ShowFragment(fragment);
}

void NotesTabWidget::Clear() {
	while (hash.count() > 0) {
		CloseNote(hash.keys().at(0));
	}
}

void NotesTabWidget::sl_Note_PropertiesChanged() {
	const Note* note = qobject_cast<Note*>(QObject::sender());
	Q_ASSERT(hash.contains(note));
	QWidget* widget = hash.value(note);
	int tabIndex = tabWidget->indexOf(widget);
	Q_ASSERT(tabIndex != -1);
	tabWidget->setTabIcon(tabIndex, note->GetIcon());
	tabWidget->setTabText(tabIndex, note->GetName());
}

QList<const Note*> NotesTabWidget::DisplayedNotes() const {
	return hash.keys();
}

Note* NotesTabWidget::CurrentNote() const {
	NoteEditWidget* noteWidget = 0;
	if (tabWidget->count() > 0) {
		noteWidget = dynamic_cast<NoteEditWidget*>(tabWidget->currentWidget());
	} else {
		return 0;
	}

	Q_ASSERT(noteWidget != 0);
	return noteWidget->CurrentNote();
}

void NotesTabWidget::SetCurrentNote(const Note* note) {
	Q_ASSERT(note != 0);
	if (hash.contains(note)) {
		QWidget* noteWidget = hash.value(note);
		tabWidget->setCurrentWidget(noteWidget);
	}
}

QList< QPair<Note*, int> > NotesTabWidget::GetState() const {
	QList< QPair<Note*, int> > list;
	for (int i = 0; i < tabWidget->count(); i++) {
		NoteEditWidget* w = dynamic_cast<NoteEditWidget*>(tabWidget->widget(i));
		QPair<Note*, int> pair (w->CurrentNote(), w->CurrentPosition());
		list.append(pair);
	}
	return list;
}

void NotesTabWidget::sl_TabWidget_CurrentChanged(int index) {
	if (index == -1) {
		emit sg_CurrentNoteChanged(0);
		return;
	}

	QWidget* widget = tabWidget->widget(index);
	NoteEditWidget* noteWidget = dynamic_cast<NoteEditWidget*>(widget);
	emit sg_CurrentNoteChanged(noteWidget->CurrentNote());
}

#if QT_VERSION < 0x040500
void NotesTabWidget::sl_CloseWidgetButton_Clicked() {
	closeTab(tabWidget->currentIndex());
}
#else
void NotesTabWidget::sl_TabWidget_TabCloseRequested(int index) {
	closeTab(index);
}
#endif

void NotesTabWidget::closeTab(int index) {
	if (index == -1) {return;}

	QWidget* widget = tabWidget->widget(index);
	Q_ASSERT(widget != 0);
	NoteEditWidget* noteWidget = dynamic_cast<NoteEditWidget*>(widget);
	const Note* note = noteWidget->CurrentNote();

	CloseNote(note);
}

QList<QAction*> NotesTabWidget::GetCurrentEditActionsList() const {
	if (tabWidget->count() == 0) {return QList<QAction*>();}

	NoteEditWidget* noteWidget = dynamic_cast<NoteEditWidget*>(tabWidget->currentWidget());
	return noteWidget->EditActionsList();
}