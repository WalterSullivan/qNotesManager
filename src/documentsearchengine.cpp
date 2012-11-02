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

#include "documentsearchengine.h"

#include "document.h"
#include "documentsearchthread.h"
#include "global.h"

#include <QEventLoop>
#include <QCoreApplication>
#include <QRegExp>
#include <QDebug>

using namespace qNotesManager;

DocumentSearchEngine::DocumentSearchEngine(QObject* parent) :
		QObject(parent),
		document(0),
		thread(new DocumentSearchThread(this)) {

	QObject::connect(thread, SIGNAL(sg_SearchResult(NoteFragment)),
					 this, SIGNAL(sg_SearchResult(NoteFragment)));
	QObject::connect(thread, SIGNAL(sg_SearchStarted()),
					 this, SIGNAL(sg_SearchStarted()));
	QObject::connect(thread, SIGNAL(sg_SearchProgress(int)),
					 this, SIGNAL(sg_SearchProgress(int)), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(sg_SearchEnded()),
					 this, SIGNAL(sg_SearchEnded()));

}

DocumentSearchEngine::~DocumentSearchEngine() {
	if (thread->isRunning()) {
		thread->Deactivate();
		thread->wait();
	}
}

bool DocumentSearchEngine::IsSearchActive() const {
	return thread->isRunning();
}

bool DocumentSearchEngine::IsQueryValid(QString query, bool useRegExp) const {
	if (!useRegExp) {
		query = QRegExp::escape(query);
	}

	QRegExp regexp (query);
	return regexp.isValid();
}

void DocumentSearchEngine::StartSearch(QString query, bool matchCase, bool searchWholeWord,
								  bool useRegexp) {
	if (thread->isRunning()) {
		WARNING("Thread is already running");
		return;
	}

	query = query.trimmed();
	if (query.isEmpty()) {
		emit sg_SearchError("Search text is empty");
		return;
	}

	if (!useRegexp) {
		query = QRegExp::escape(query);
	}

	if (searchWholeWord) {
		query.prepend("\\b").append("\\b");
	}

	Qt::CaseSensitivity cs = matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive;

	QRegExp regexp (query, cs);

	if (!regexp.isValid()) {
		emit sg_SearchError("Regexp is invalid");
		return;
	}

	const QList<Note*> notes = document->GetNotesList();

	thread->SetRegexp(regexp);
	foreach (Note* n, notes) {
		thread->AddNote(n);
	}

	thread->start();
}

void DocumentSearchEngine::StopSearch() {
	if (thread->isRunning()) {
		thread->Deactivate();
		thread->wait();
	}
}

void DocumentSearchEngine::sl_Document_NoteDeleted(Note* n) {
	// If thread is executing search in note n then wait until it ends, block user input
	while(thread->CurrentNote() == n) {
		QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	}

	thread->RemoveNote(n);
}

void DocumentSearchEngine::sl_Document_NoteAdded(Note* n) {
	thread->AddNote(n);
}

void DocumentSearchEngine::SetTargetDocument(Document* doc) {
	if (IsSearchActive()) {
		StopSearch();
	}

	if (document != 0) {
		QObject::disconnect(document, 0, this, 0);
	}

	document = doc;

	if (document != 0) {
		QObject::connect(document, SIGNAL(sg_ItemRegistered(Note*)),
						 this, SLOT(sl_Document_NoteAdded(Note*)));
		QObject::connect(document, SIGNAL(sg_ItemUnregistered(Note*)),
						 this, SLOT(sl_Document_NoteDeleted(Note*)));
		QObject::connect(document, SIGNAL(destroyed()),
						 this, SLOT(sl_Document_Destroyed()));
	}
}

void DocumentSearchEngine::sl_Document_Destroyed() {
	// In case if we forget to null target document
	document = 0;

	thread->ClearNotesList();
}
