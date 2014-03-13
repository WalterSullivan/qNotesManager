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

#include "documentsearchthread.h"

#include "note.h"
#include "global.h"

#include <QDebug>

using namespace qNotesManager;

DocumentSearchThread::DocumentSearchThread(QObject *parent) :
		QThread(parent),
		regexp(QRegExp()),
		isActive(false),
		currentNote(0),
		primarySearchQueueSize(0),
		processedNotesCount(0)
{
}

bool DocumentSearchThread::IsActive() const {
	QReadLocker locker(&isActiveLock);
	return isActive;
}

void DocumentSearchThread::SetActive(bool a) {
	QWriteLocker locker(&isActiveLock);
	if (isActive == a) {return;}

	isActive = a;
}

void DocumentSearchThread::Deactivate() {
	QWriteLocker locker(&isActiveLock);
	if (!isActive) {return;}

	isActive = false;
}

void DocumentSearchThread::SetCurrentNote(const Note* n) {
	QWriteLocker locker(&listLock);
	currentNote = n;
}

const Note* DocumentSearchThread::CurrentNote() const {
	QReadLocker locker(&listLock);
	return currentNote;
}

void DocumentSearchThread::run() {
	if (!regexp.isValid()) {
		WARNING("DocumentSearchThread::run: regexp is invalid");
		return;
	}
	if (regexp == QRegExp()) {
		WARNING("DocumentSearchThread::run: regexp is empty");
		return;
	}

	const int symbolsForSample = 40;
	int textMatchStart = -1;
	int textMatchLength = -1;
	int currentPos = 0;

	listLock.lockForRead();
	primarySearchQueueSize = searchQueue.size();
	listLock.unlock();
	processedNotesCount = 0;

	SetActive(true);
	emit sg_SearchStarted();

	while(true) {
		listLock.lockForRead();
		if (searchQueue.isEmpty()) {
			listLock.unlock();
			break;
		}
		listLock.unlock();

		if (!IsActive()) {
			break;
		}

		listLock.lockForWrite();
		const Note* n = searchQueue.takeFirst();
		listLock.unlock();

		// If note's text document was not initialized, ask to initialize it in main
		// thread (!important) and put it back in the queue
		if (!n->TextDocumentInitialized()) {
			QObject::connect(this, SIGNAL(sg_TextDocumentInitRequest()),
							 n, SLOT(sl_InitTextDocument()), Qt::QueuedConnection);
			sg_TextDocumentInitRequest();
			QObject::disconnect(this, 0, n, 0);

			listLock.lockForWrite();
			searchQueue.push_front(n);
			listLock.unlock();
			continue;
		}


		SetCurrentNote(n);

		// TODO: fix this mess

		// Search in caption
		// FIXME: fix situation when capturedText.length() > symbolsForSample
		currentPos = 0;
		QString text = currentNote->GetName();
		while (true) {
			textMatchStart = regexp.indexIn(text, currentPos);
			textMatchLength = regexp.matchedLength();
			if (textMatchStart != -1 && textMatchLength != -1) {
				const int textLength = text.length();
				const QString capturedText = regexp.capturedTexts().at(0);
				const int appendSymbols = (symbolsForSample - capturedText.length()) / 2;
				int sampleStart = (textMatchStart - appendSymbols) >= 0
								  ? (textMatchStart - appendSymbols)
								  : 0;
				int sampleLength = (textMatchStart + textMatchLength + appendSymbols - 1) < textLength
								  ? (textMatchStart + textMatchLength + appendSymbols - 1)
								  : textLength;
				const QString sample = text.mid(sampleStart, sampleLength);
				const int sampleMatchStart = textMatchStart - sampleStart;//sample.indexOf(capturedText);



				NoteFragment f(currentNote, NoteFragment::CaptionFragment, textMatchStart,
							   textMatchLength, sample, sampleMatchStart, capturedText.length());
				emit sg_SearchResult(f);
			} else {break;}

			currentPos = textMatchStart + 1;
		}

		// Search in author field
		currentPos = 0;
		text = currentNote->GetAuthor();
		while (true) {
			textMatchStart = regexp.indexIn(text, currentPos);
			textMatchLength = regexp.matchedLength();
			if (textMatchStart != -1 && textMatchLength != -1) {
				const int textLength = text.length();
				const QString capturedText = regexp.capturedTexts().at(0);
				const int appendSymbols = (symbolsForSample - capturedText.length()) / 2;
				int sampleStart = (textMatchStart - appendSymbols) >= 0 ? (textMatchStart - appendSymbols) : 0;
				int sampleLength = (textMatchStart + textMatchLength + appendSymbols - 1) < textLength ?
								   (textMatchStart + textMatchLength + appendSymbols - 1) : textLength;
				const QString sample = text.mid(sampleStart, sampleLength);
				const int sampleMatchStart = textMatchStart - sampleStart;//sample.indexOf(capturedText);



				NoteFragment f(currentNote, NoteFragment::AuthorFragment, textMatchStart,
							   textMatchLength, sample, sampleMatchStart, capturedText.length());
				emit sg_SearchResult(f);
			} else {break;}

			currentPos = textMatchStart + 1;
		}

		// Search in source field
		currentPos = 0;
		text = currentNote->GetSource();
		while (true) {
			textMatchStart = regexp.indexIn(text, currentPos);
			textMatchLength = regexp.matchedLength();
			if (textMatchStart != -1 && textMatchLength != -1) {
				const int textLength = text.length();
				const QString capturedText = regexp.capturedTexts().at(0);
				const int appendSymbols = (symbolsForSample - capturedText.length()) / 2;
				int sampleStart = (textMatchStart - appendSymbols) >= 0 ? (textMatchStart - appendSymbols) : 0;
				int sampleLength = (textMatchStart + textMatchLength + appendSymbols - 1) < textLength ?
								   (textMatchStart + textMatchLength + appendSymbols - 1) : textLength;
				const QString sample = text.mid(sampleStart, sampleLength);
				const int sampleMatchStart = textMatchStart - sampleStart;//sample.indexOf(capturedText);



				NoteFragment f(currentNote, NoteFragment::SourceFragment, textMatchStart,
							   textMatchLength, sample, sampleMatchStart, capturedText.length());
				emit sg_SearchResult(f);
			} else {break;}

			currentPos = textMatchStart + 1;
		}

		// Search in comment field
		currentPos = 0;
		text = currentNote->GetComment();
		while (true) {
			textMatchStart = regexp.indexIn(text, currentPos);
			textMatchLength = regexp.matchedLength();
			if (textMatchStart != -1 && textMatchLength != -1) {
				const int textLength = text.length();
				const QString capturedText = regexp.capturedTexts().at(0);
				const int appendSymbols = (symbolsForSample - capturedText.length()) / 2;
				int sampleStart = (textMatchStart - appendSymbols) >= 0 ? (textMatchStart - appendSymbols) : 0;
				int sampleLength = (textMatchStart + textMatchLength + appendSymbols - 1) < textLength ?
								   (textMatchStart + textMatchLength + appendSymbols - 1) : textLength;
				const QString sample = text.mid(sampleStart, sampleLength);
				const int sampleMatchStart = textMatchStart - sampleStart;//sample.indexOf(capturedText);



				NoteFragment f(currentNote, NoteFragment::CommentFragment, textMatchStart,
							   textMatchLength, sample, sampleMatchStart, capturedText.length());
				emit sg_SearchResult(f);
			} else {break;}

			currentPos = textMatchStart + 1;
		}

		// Search in text
		currentPos = 0;
		const QString elide = "...";
		text = currentNote->GetText();
		while (true) {
			// FIXME: fix situation when capturedText.length() > symbolsForSample
			textMatchStart = regexp.indexIn(text, currentPos);
			textMatchLength = regexp.matchedLength();
			if (textMatchStart != -1 && textMatchLength != -1) {

				const int textLength = text.length();
				const QString capturedText = regexp.capturedTexts().at(0);
				const int appendSymbols = (symbolsForSample - capturedText.length() -
										   (elide.length()*2)) / 2;
				int sampleStart = (textMatchStart - appendSymbols) >= 0 ? (textMatchStart - appendSymbols) : 0;
				int sampleLength = (textMatchStart + textMatchLength + appendSymbols - 1) < textLength ?
								   (appendSymbols + textMatchLength + appendSymbols) : textLength;

				QString sample =
						text.mid(sampleStart, sampleLength).append(elide).prepend(elide);
				sample.replace(QRegExp("\n"), " ");
				const int sampleMatchStart = textMatchStart - sampleStart + elide.length();


				NoteFragment f(currentNote, NoteFragment::TextFragment, textMatchStart, textMatchLength, sample,
							   sampleMatchStart, capturedText.length());
				emit sg_SearchResult(f);


			} else {
				break;
			}
			currentPos = textMatchStart + 1;
		}

		processedNotesCount++;
		int progress = 0;
		if (primarySearchQueueSize != 0) {
			progress = (int)(((qint64)processedNotesCount * 100) / primarySearchQueueSize);
		}
		if (progress > 100) {progress = 100;}
		emit sg_SearchProgress(progress);
	}


	regexp = QRegExp();
	emit sg_SearchEnded();
	SetActive(false);
	SetCurrentNote(0);
}

void DocumentSearchThread::SetRegexp(const QRegExp& regexp) {
	if (isRunning()) {return;}

	this->regexp = regexp;
}

void DocumentSearchThread::AddNote(const Note* n) {
	QWriteLocker locker(&listLock);
	searchQueue.append(n);
}

void DocumentSearchThread::RemoveNote(const Note* n) {
	QWriteLocker locker(&listLock);
	searchQueue.removeOne(n);
}

void DocumentSearchThread::ClearNotesList() {
	QWriteLocker locker(&listLock);
	searchQueue.clear();
}
