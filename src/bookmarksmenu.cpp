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

#include "bookmarksmenu.h"

#include "document.h"
#include "note.h"

#include <QDebug>

using namespace qNotesManager;

BookmarksMenu::BookmarksMenu(const QString& title, QWidget *parent) : QMenu(title, parent) {
	currentDocument = nullptr;
	currentNote = nullptr;

	addBookmarkAction = new QAction(QIcon(":gui/star-plus"), "Add to bookmarks", this);
	addBookmarkAction->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_B));
	QObject::connect(addBookmarkAction, SIGNAL(triggered()),
					 this, SLOT(sl_AddBookmarkAction_Triggered()));
	addBookmarkAction->setEnabled(false);

	removeBookmarkAction = new QAction(QIcon(":/gui/star-minus"), "Remove from bookmarks", this);
	removeBookmarkAction->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_B));
	QObject::connect(removeBookmarkAction, SIGNAL(triggered()),
					 this, SLOT(sl_RemoveBookmarkAction_Triggered()));
	removeBookmarkAction->setEnabled(false);

	addAction(addBookmarkAction);
	addAction(removeBookmarkAction);
	addSeparator();

	QObject::connect(this, SIGNAL(triggered(QAction*)),
					 this, SLOT(sl_ActionTriggered(QAction*)));

	menuAction()->setEnabled(false);
}

void BookmarksMenu::SetDocument(Document* document) {
	if (currentDocument) {
		QObject::disconnect(currentDocument, 0, this, 0);
	}

	currentDocument = document;

	if (currentDocument == nullptr) {

	} else {
		QObject::connect(currentDocument, SIGNAL(sg_BookmarksListChanged()),
						 this, SLOT(sl_Document_BookmarksListChanged()));
	}

	menuAction()->setEnabled(currentDocument != nullptr);
	updateMenu();
}

void BookmarksMenu::sl_CurrentNoteChanged(Note* note){
	currentNote = note;

	if (currentNote == nullptr || currentDocument == nullptr) {
		addBookmarkAction->setEnabled(false);
		removeBookmarkAction->setEnabled(false);
	} else {
		if (currentDocument &&
			currentDocument->IsBookmark(currentNote)) {
			addBookmarkAction->setEnabled(false);
			removeBookmarkAction->setEnabled(true);
		} else {
			addBookmarkAction->setEnabled(true);
			removeBookmarkAction->setEnabled(false);
		}
	}
}

void BookmarksMenu::sl_AddBookmarkAction_Triggered() {
	if (currentNote == nullptr) {return;}
	if (currentDocument == nullptr) {return;}

	currentDocument->AddBookmark(currentNote);
}

void BookmarksMenu::sl_RemoveBookmarkAction_Triggered() {
	if (currentNote == nullptr) {return;}
	if (currentDocument == nullptr) {return;}

	currentDocument->RemoveBookmark(currentNote);
}

void BookmarksMenu::sl_ActionTriggered(QAction* action) {
	if (action == addBookmarkAction || action == removeBookmarkAction) {return;}

	Note* note = links[action];
	if (note == nullptr) {return;}

	emit sg_OpenBookmark(note);
}

void BookmarksMenu::sl_Document_BookmarksListChanged() {
	updateMenu();
}

void BookmarksMenu::updateMenu() {
	QList<QAction*> actions = this->actions();
	for (int i = 3; i < actions.size(); i++) {
		QAction* action = actions[i];
		removeAction(action);
		delete action;
	}
	links.clear();

	if (currentDocument == nullptr) {return;}

	for (int i = 0; i < currentDocument->GetBookmarksCount(); i++) {
		Note* note = currentDocument->GetBookmark(i);

		QAction* action = new QAction(note->GetIcon(), note->GetName(), nullptr);

		addAction(action);
		links.insert(action, note);
	}

	if (currentNote == nullptr) {
		addBookmarkAction->setEnabled(false);
		removeBookmarkAction->setEnabled(false);
	} else {
		if (currentDocument &&
			currentDocument->IsBookmark(currentNote)) {
			addBookmarkAction->setEnabled(false);
			removeBookmarkAction->setEnabled(true);
		} else {
			addBookmarkAction->setEnabled(true);
			removeBookmarkAction->setEnabled(false);
		}
	}
}
