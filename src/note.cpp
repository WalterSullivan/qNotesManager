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

#include "note.h"

#include "application.h"
#include "document.h"
#include "textdocument.h"
#include "global.h"
#include "cachedimagefile.h"
#include "cachedfile.h"

#include <QDebug>

using namespace qNotesManager;

Note::Note(QString _name) :
		AbstractFolderItem(AbstractFolderItem::Type_Note),
		name(_name),
		text(QString()),
		creationDate(QDateTime::currentDateTime()),
		modificationDate(QDateTime::currentDateTime()),
		textDate(QDateTime()),
		iconID(""),
		author(QString()),
		source(QString()),
		comment(QString()),
		defaultForeColor(QColor(0, 0, 0, 255)),
		defaultBackColor(QColor(0, 0, 0, 0)),
		nameForeColor(defaultForeColor),
		nameBackColor(defaultBackColor),
		locked(false),
		document(new TextDocument(this)),
		textUpdateTimer(this),
		cachedHtml(QString()),
		textDocumentInitialized(true),
		Tags(this),
		IsTagsListInitializationInProgress(false){
	QObject::connect(&Tags, SIGNAL(sg_ItemAboutToBeAdded(Tag*)),
					 this, SIGNAL(sg_TagAboutToBeAdded(Tag*)));
	QObject::connect(&Tags, SIGNAL(sg_ItemAdded(Tag*)),
					 this, SIGNAL(sg_TagAdded(Tag*)));
	QObject::connect(&Tags, SIGNAL(sg_ItemAboutToBeRemoved(Tag*)),
					 this, SIGNAL(sg_TagAboutToBeRemoved(Tag*)));
	QObject::connect(&Tags, SIGNAL(sg_ItemRemoved(Tag*)),
					 this, SIGNAL(sg_TagRemoved(Tag*)));

	QObject::connect(&Tags, SIGNAL(sg_ItemAdded(Tag*)),
					 this, SLOT(sl_TagsCollectionModified(Tag*)));
	QObject::connect(&Tags, SIGNAL(sg_ItemRemoved(Tag*)),
					 this, SLOT(sl_TagsCollectionModified(Tag*)));


	// textUpdateTimer updates cached text in 'text' variable in 2 after document've been edited
	textUpdateTimer.setInterval(2000);
	textUpdateTimer.setSingleShot(true);
	QObject::connect(&textUpdateTimer, SIGNAL(timeout()), this, SLOT(sl_TextUpdateTimer_Timeout()));

	if (name.isEmpty()) {name = "New note";}

	QObject::connect(document, SIGNAL(contentsChanged()), this, SLOT(sl_DocumentChanged()));

	document->DummyImagesProvider = Application::I();
}

Note::~Note() {
	Tags.Clear();
	delete document;

	foreach (CachedFile* file, attachedFiles) {
		delete file;
	}
	attachedFiles.clear();
}

QString Note::GetName() const {
	QReadLocker locker(&lock);
	return name;
}

void Note::SetName (QString s) {
	lock.lockForWrite();
	if (name == s) {
		lock.unlock();
		return;
	}

	name = s;
	lock.unlock();

	emit sg_VisualPropertiesChanged();
	emit sg_PropertyChanged();
	onChange();
}

QDateTime Note::GetCreationDate() const {
	return creationDate;
}

QDateTime Note::GetModificationDate() const {
	return modificationDate;
}

QPixmap Note::GetIcon() const {
	if (Application::I()->CurrentDocument() == 0) {
		return QPixmap();
	}
	QPixmap icon = Application::I()->CurrentDocument()->GetItemIcon(iconID);
	return icon;
}

void Note::SetIconID(QString id) {
	if (iconID == id) {
		return;
	}
	if (id.isEmpty()) {
		WARNING("New icon id is empty");
		return;
	}

	iconID = id;

	emit sg_VisualPropertiesChanged();
	emit sg_PropertyChanged();
	onChange();
}

QString Note::GetIconID() const {
	return iconID;
}

QString Note::GetText() const {
	if (!cachedHtml.isNull()) {
		sl_InitTextDocument();
	}

	QReadLocker locker(&lock);

	return text;
}

void Note::SetText(QString t) {
	if (text == t) {
		return;
	}

	GetTextDocument()->setPlainText(t);

	emit sg_TextChanged();
}

void Note::SetHtml(QString t) {
	GetTextDocument()->setHtml(t);

	emit sg_TextChanged();
}

QColor Note::GetNameForeColor() const {
	return nameForeColor;
}

void Note::SetNameForeColor(QColor c) {
	if (!c.isValid()) {
		WARNING("New color is invalid");
		return;
	}

	if (nameForeColor == c) {
		return;
	}

	nameForeColor = c;

	emit sg_VisualPropertiesChanged();
	emit sg_PropertyChanged();
	onChange();
}

QColor Note::GetNameBackColor() const {
	return nameBackColor;
}

void Note::SetNameBackColor(QColor c) {
	if (!c.isValid()) {
		WARNING("New color is invalid");
		return;
	}

	if (nameBackColor == c) {
		return;
	}

	nameBackColor = c;

	emit sg_VisualPropertiesChanged();
	emit sg_PropertyChanged();
	onChange();
}

QColor Note::GetDefaultForeColor() const {
	return defaultForeColor;
}

QColor Note::GetDefaultBackColor() const {
	return defaultBackColor;
}

bool Note::IsLocked() const {
	QReadLocker locker(&lock);
	return locked;
}

void Note::SetLocked(bool e) {
	lock.lockForWrite();
	if (locked == e) {
		lock.unlock();
		return;
	}

	locked = e;

	lock.unlock();

	emit sg_VisualPropertiesChanged();
	emit sg_PropertyChanged();
	onChange();
}

QString Note::GetAuthor() const {
	QReadLocker locker(&lock);
	return author;
}

void Note::SetAuthor(QString a) {
	lock.lockForWrite();
	if (author == a) {
		lock.unlock();
		return;
	}

	author = a;
	lock.unlock();

	emit sg_PropertyChanged();
	onChange();
}

QString Note::GetSource() const {
	QReadLocker locker(&lock);
	return source;
}

void Note::SetSource(QString s) {
	lock.lockForWrite();
	if (source == s) {
		lock.unlock();
		return;
	}

	source = s;
	lock.unlock();

	emit sg_PropertyChanged();
	onChange();
}

QDateTime Note::GetTextCreationDate() const {
	return textDate;
}

void Note::SetTextCreationDate(QDateTime d) {
	if (textDate == d) {
		return;
	}

	textDate = d;

	emit sg_TextDateChanged();
	emit sg_PropertyChanged();
	onChange();
}

QString Note::GetComment() const {
	QReadLocker locker(&lock);
	return comment;
}

void Note::SetComment(QString c) {
	lock.lockForWrite();
	if (comment == c) {
		lock.unlock();
		return;
	}

	comment = c;
	lock.unlock();

	emit sg_PropertyChanged();
	onChange();
}

void Note::sl_DocumentChanged() {
	textUpdateTimer.start();

	onChange();
}

TextDocument* Note::GetTextDocument() const {
	if (!cachedHtml.isNull()) {
		sl_InitTextDocument();
	}

	return document;
}

bool Note::TextDocumentInitialized() const {
	return textDocumentInitialized;
}

int Note::GetAttachedFilesCount() const {
	return attachedFiles.count();
}

void Note::AttachFile(CachedFile* file) {
	if (file == 0) {return;}
	if (file->Size() == 0) {return;}
	if (attachedFiles.contains(file)) {return;}

	attachedFiles.append(file);
	emit sg_PropertyChanged();
	onChange();
}

void Note::RemoveAttachedFile(CachedFile* file) {
	if (file == 0) {return;}
	if (!attachedFiles.contains(file)) {return;}

	attachedFiles.removeAll(file);
	emit sg_PropertyChanged();
	onChange();
}

void Note::RemoveAttachedFile(const int index) {
	if (index < 0 || index >= attachedFiles.count()) {return;}

	attachedFiles.removeAt(index);
	emit sg_PropertyChanged();
	onChange();
}

CachedFile* Note::GetAttachedFile(int index) const {
	if (index < 0 || index >= attachedFiles.count()) {return 0;}

	return attachedFiles[index];
}

void Note::onChange() {
	QDateTime now = QDateTime::currentDateTime();
	bool modifyDateChanged = modificationDate.date() != now.date();
	modificationDate = now;

	if (modifyDateChanged) {
		emit sg_ModifyDateChanged();
	}
	emit sg_DataChanged();
}

void Note::sl_TagsCollectionModified(Tag*) {
	if (IsTagsListInitializationInProgress) {return;}
	emit sg_PropertyChanged();
	onChange();
}

void Note::sl_TextUpdateTimer_Timeout() {
	QWriteLocker locker(&lock);
	text = document->toPlainText();
}

void Note::sl_InitTextDocument() const {
	if (textDocumentInitialized) {return;}

	QWriteLocker locker(&lock);

	document->blockSignals(true);
		if (!cachedHtml.isNull()) {
			document->setHtml(cachedHtml);
		}
		text = document->toPlainText();
	document->blockSignals(false);
	textDocumentInitialized = true;

	cachedHtml = QString();
}
