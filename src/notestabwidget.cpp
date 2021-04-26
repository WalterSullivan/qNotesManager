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
#include "global.h"

#include <QBoxLayout>
#include <QApplication>

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

	closeTabAction = new QAction("Close tab", this);
	this->addAction(closeTabAction);
	closeTabAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
	closeTabAction->setShortcutContext(Qt::WindowShortcut);
	QObject::connect(closeTabAction, SIGNAL(triggered()),
					 this, SLOT(sl_CloseCurrentTab()));
}

void NotesTabWidget::OpenNote(Note* note, int position, bool newTab) {
	(void)newTab;

	if (!note) {
		WARNING("Null pointer recieved");
		return;
	}

	if (hash.contains(note)) {
		QWidget* noteWidget = hash.value(note);
		qobject_cast<NoteEditWidget*>(noteWidget)->FocusTextEdit();
		tabWidget->setCurrentWidget(noteWidget);
		return;
	}

	bool showWaitCursor = !note->TextDocumentInitialized();
	if (showWaitCursor) {
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	}
	NoteEditWidget* w = new NoteEditWidget(note);
	if (showWaitCursor) {
		QApplication::restoreOverrideCursor();
	}
	hash.insert(note, w);
	QObject::connect(note, SIGNAL(sg_VisualPropertiesChanged()),
					 this, SLOT(sl_Note_PropertiesChanged()));


	tabWidget->addTab(w, note->GetIcon(), cropStringForTabCaption(note->GetName()));
	tabWidget->setCurrentWidget(w);
	w->FocusTextEdit();
	w->ScrollTo(position);
}

void NotesTabWidget::CloseNote(const Note* n) {
	if (!n) {
		WARNING("Null pointer recieved");
		return;
	}

	if (!hash.contains(n)) {
		WARNING("Specified note is not open");
		return;
	}

	QWidget* widget = hash.take(n);
	int tabIndex = tabWidget->indexOf(widget);

	if (tabIndex == -1) {
		WARNING("Could not find associated widget");
		return;
	}

	QObject::disconnect(n, 0, this, 0);
	tabWidget->removeTab(tabIndex);
	delete widget;
}

void NotesTabWidget::ShowFragment(const NoteFragment& fragment) {
	if (!fragment.NotePtr) {
		WARNING("Null pointer recieved");
		return;
	}

	OpenNote(const_cast<Note*>(fragment.NotePtr));

	NoteEditWidget* noteWidget =
			dynamic_cast<NoteEditWidget*>(hash.value(fragment.NotePtr));

	noteWidget->ShowFragment(fragment);
}

void NotesTabWidget::Clear() {
	while (hash.count() > 0) {
		CloseNote(hash.keys().at(0));
	}
}

void NotesTabWidget::sl_Note_PropertiesChanged() {
	const Note* note = qobject_cast<Note*>(QObject::sender());
	if (note == 0) {
		WARNING("Could not find sender note");
		return;
	}

	if (!hash.contains(note)) {
		WARNING("Sender note is not registered");
		QObject::disconnect(note, 0, this, 0);
		return;
	}

	QWidget* widget = hash.value(note);
	int tabIndex = tabWidget->indexOf(widget);

	if (tabIndex == -1) {
		WARNING("Could not find associated widget");
		return;
	}

	tabWidget->setTabIcon(tabIndex, note->GetIcon());
	tabWidget->setTabText(tabIndex, cropStringForTabCaption(note->GetName()));
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

	if (noteWidget == 0) {
		WARNING("Casting error");
		return 0;
	}

	return noteWidget->CurrentNote();
}

void NotesTabWidget::SetCurrentNote(const Note* note) {
	if (!note) {
		WARNING("Null pointer recieved");
		return;
	}

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
	if (index == -1) {
		WARNING("Wrong tab index");
		return;
	}

	QWidget* widget = tabWidget->widget(index);
	if (widget == 0) {
		WARNING("Can't find widget by index");
		return;
	}
	NoteEditWidget* noteWidget = dynamic_cast<NoteEditWidget*>(widget);
	const Note* note = noteWidget->CurrentNote();

	CloseNote(note);
}

void NotesTabWidget::sl_CloseCurrentTab() {
	int currentTabIndex = tabWidget->currentIndex();
	if (currentTabIndex != -1) {
		closeTab(currentTabIndex);
	}
}

QList<QAction*> NotesTabWidget::GetCurrentEditActionsList() const {
	if (tabWidget->count() == 0) {return QList<QAction*>();}

	NoteEditWidget* noteWidget = dynamic_cast<NoteEditWidget*>(tabWidget->currentWidget());
	return noteWidget->EditActionsList();
}

QString NotesTabWidget::cropStringForTabCaption(QString text) const {
	const int maxTabCaptionLength = 12;

	if (text.length() > maxTabCaptionLength) {
		text = text.mid(0, maxTabCaptionLength).append("...");
	}

	return text;
}
