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
#include "boibuffer.h"

#include <QBuffer>
#include <QFileInfo>
#include <QDebug>


using namespace qNotesManager;

Note::Note(QString _name) :
		AbstractFolderItem(AbstractFolderItem::Type_Note),
		name(_name),
		creationDate(QDateTime::currentDateTime()),
		modificationDate(QDateTime::currentDateTime()),
		textDate(QDateTime()),
		text(QString("")),
		defaultForeColor(QColor(0, 0, 0, 255)),
		defaultBackColor(QColor(0, 0, 0, 0)),
		nameForeColor(defaultForeColor),
		nameBackColor(defaultBackColor),
		locked(false),
		document(new TextDocument(this)),
		Tags(this) {
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


	QObject::connect(document, SIGNAL(contentsChanged()), this, SLOT(sl_DocumentChanged()));

	iconID = ":/standard/note";

	if (name.isEmpty()) {name = "New note";}
}

Note::~Note() {
	Tags.Clear();
	delete document;
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
	onChange();
}

QDateTime Note::GetCreationDate() const {
	return creationDate;
}

QDateTime Note::GetModificationDate() const {
	return modificationDate;
}

QPixmap Note::GetIcon() const {
	QPixmap icon = Application::I()->CurrentDocument()->GetItemIcon(iconID);
	return icon;
}

void Note::SetIcon(QString id) {
	if (iconID == id) {
		return;
	}
	Q_ASSERT(!id.isEmpty());

	iconID = id;

	emit sg_VisualPropertiesChanged();
	onChange();
}

QString Note::GetIconID() const {
	return iconID;
}

QString Note::GetText() const {
	QReadLocker locker(&lock);
	return text;
}

void Note::SetText(QString t) {
	if (text == t) {
		return;
	}

	document->setPlainText(t);

	emit sg_TextChanged();
	onChange();
}

void Note::SetHtml(QString t) {
	document->setHtml(t);

	emit sg_TextChanged();
	onChange();
}

QColor Note::GetNameForeColor() const {
	return nameForeColor;
}

void Note::SetNameForeColor(QColor c) {
	if (!c.isValid()) {return;}

	if (nameForeColor == c) {
		return;
	}

	nameForeColor = c;

	emit sg_VisualPropertiesChanged();
	onChange();
}

QColor Note::GetNameBackColor() const {
	return nameBackColor;
}

void Note::SetNameBackColor(QColor c) {
	if (!c.isValid()) {return;}

	if (nameBackColor == c) {
		return;
	}

	nameBackColor = c;

	emit sg_VisualPropertiesChanged();
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

	onChange();
}

void Note::sl_DocumentChanged() {
	lock.lockForWrite();
	text = document->toPlainText();
	lock.unlock();

	onChange();
}

TextDocument* Note::GetTextDocument() const {
	return document;
}

// NOT thread-safe
void Note::Serialize(const int version, BOIBuffer& stream, const QHash<QString, quint32>& hash) const {
	(void)version;

	const QByteArray w_captionArray = name.toUtf8();
	const quint32 w_captionSize = w_captionArray.size();
	const QByteArray w_textArray = document->toHtml().toUtf8();
	const quint32 w_textSize = w_textArray.size();
	const quint32 w_creationDate = creationDate.toTime_t();
	const quint32 w_modificationDate = modificationDate.toTime_t();
	const quint32 w_textDate = textDate.isValid() ? textDate.toTime_t() : 0;
	const quint32 w_iconID = hash.value(iconID);
	const QByteArray w_authorArray = author.toUtf8();
	const quint32 w_authorSize = w_authorArray.size();
	const QByteArray w_sourceArray = source.toUtf8();
	const quint32 w_sourceSize = w_sourceArray.size();
	const QByteArray w_commentArray = comment.toUtf8();
	const quint32 w_commentSize = w_commentArray.size();
	const quint32 w_backColor = nameBackColor.rgba();
	const quint32 w_foreColor = nameForeColor.rgba();
	const quint8 w_locked = (quint8)locked;

	// Write images to temporary buffer
	const QByteArray imagesByteArray = document->SerializeImageResources();


	const quint32 w_imagesArraySize = imagesByteArray.size();
	const quint32 w_itemSize =	sizeof(w_captionSize) +
								w_captionSize +
								sizeof(w_textSize) +
								w_textSize +
								sizeof(w_creationDate) +
								sizeof(w_modificationDate) +
								sizeof(w_textDate) +
								sizeof(w_iconID) +
								sizeof(w_authorSize) +
								w_authorSize +
								sizeof(w_sourceSize) +
								w_sourceSize +
								sizeof(w_commentSize) +
								w_commentSize +
								sizeof(w_backColor) +
								sizeof(w_foreColor) +
								sizeof(w_locked) +
								sizeof(w_imagesArraySize) +
								w_imagesArraySize;
	qint64 result = 0;
	result = stream.write(w_itemSize);
	result = stream.write(w_captionSize);
	result = stream.write(w_captionArray.constData(),	w_captionSize);
	result = stream.write(w_textSize);
	result = stream.write(w_textArray.constData(),		w_textSize);
	result = stream.write(w_creationDate);
	result = stream.write(w_modificationDate);
	result = stream.write(w_textDate);
	result = stream.write(w_authorSize);
	result = stream.write(w_authorArray.constData(),	w_authorSize);
	result = stream.write(w_sourceSize);
	result = stream.write(w_sourceArray.constData(),	w_sourceSize);
	result = stream.write(w_commentSize);
	result = stream.write(w_commentArray.constData(),	w_commentSize);
	result = stream.write(w_iconID);
	result = stream.write(w_backColor);
	result = stream.write(w_foreColor);
	result = stream.write(w_locked);
	result = stream.write(w_imagesArraySize);
	result = stream.write(imagesByteArray);
}

/* static */
// NOT thread-safe
Note* Note::Deserialize(const int version, BOIBuffer& stream, const QHash<quint32, QString>& hash) {
	(void)version;

	qint64 bytesRead = 0;

	quint32 r_itemSize = 0;
	bytesRead = stream.read(r_itemSize);

	const qint64 dataStartPos = stream.pos();

	quint32 r_captionSize = 0;
	bytesRead = stream.read(r_captionSize);
	QByteArray r_captionArray(r_captionSize, 0x0);
	bytesRead = stream.read(r_captionArray.data(), r_captionSize);
	quint32 r_textSize = 0;
	bytesRead = stream.read(r_textSize);
	QByteArray r_textArray(r_textSize, 0x0);
	bytesRead = stream.read(r_textArray.data(), r_textSize);
	quint32 r_creationDate = 0;
	bytesRead = stream.read(r_creationDate);
	quint32 r_modificationDate = 0;
	bytesRead = stream.read(r_modificationDate);
	quint32 r_textDate = 0;
	bytesRead = stream.read(r_textDate);
	quint32 r_authorSize = 0;
	bytesRead = stream.read(r_authorSize);
	QByteArray r_authorArray(r_authorSize, 0x0);
	bytesRead = stream.read(r_authorArray.data(), r_authorSize);
	quint32 r_sourceSize = 0;
	bytesRead = stream.read(r_sourceSize);
	QByteArray r_sourceArray(r_sourceSize, 0x0);
	bytesRead = stream.read(r_sourceArray.data(), r_sourceSize);
	quint32 r_commentSize = 0;
	bytesRead = stream.read(r_commentSize);
	QByteArray r_commentArray(r_commentSize, 0x0);
	bytesRead = stream.read(r_commentArray.data(),			r_commentSize);
	quint32 r_iconID = 0;
	bytesRead = stream.read(r_iconID);
	quint32 r_backColor = 0;
	bytesRead = stream.read(r_backColor);
	quint32 r_foreColor = 0;
	bytesRead = stream.read(r_foreColor);
	quint8 r_locked = 0;
	bytesRead = stream.read(r_locked);
	quint32 r_imagesListSize = 0;
	bytesRead = stream.read(r_imagesListSize);

	QMap<QString, QImage> images;
	if (r_imagesListSize > 0) {
		quint32 imagesSize = 0;

		while(imagesSize < r_imagesListSize) {
			quint32 r_imageNameSize = 0;
			bytesRead = stream.read(r_imageNameSize);
			QByteArray r_imageName(r_imageNameSize, 0x0);
			bytesRead = stream.read(r_imageName.data(), r_imageNameSize);

			quint32 imageFormatSize = 0;
			bytesRead = stream.read(imageFormatSize);
			QByteArray imageFormat(imageFormatSize, 0x0);
			bytesRead = stream.read(imageFormat.data(), imageFormatSize);

			quint32 r_imageArraySize = 0;
			bytesRead = stream.read(r_imageArraySize);
			QByteArray r_imageArray(r_imageArraySize, 0x0);
			bytesRead = stream.read(r_imageArray.data(), r_imageArraySize);

			QImage image = QImage::fromData(r_imageArray, QString(imageFormat).toStdString().c_str());
			if (!image.isNull()) {
#ifdef DEBUG
				qDebug() << "Image " << r_imageName << ".Suffix: " << imageFormat << " .loaded";
#endif
				image.setText("FORMAT", QString(imageFormat));
				images.insert(r_imageName, image);
			} else {
				qWarning("Image %s. Suffix: %s not loaded", qPrintable(QString(r_imageName)),
						 qPrintable(QString(imageFormat)) );
			}


			imagesSize +=	sizeof(r_imageNameSize) +
							r_imageNameSize +
							sizeof(imageFormatSize) +
							imageFormatSize +
							sizeof(r_imageArraySize) +
							r_imageArraySize;
		}
	}

	const quint32 bytesToSkip = r_itemSize - (stream.pos() - dataStartPos);

	if (bytesToSkip != 0) {
		// If block has more data in case of newer file version.
		stream.seek(stream.pos() + bytesToSkip);
	}


	Note* note = new Note("");
	note->name = r_captionArray;
	note->creationDate = QDateTime::fromTime_t(r_creationDate);
	note->modificationDate = QDateTime::fromTime_t(r_modificationDate);
	note->textDate = r_textDate == 0 ? QDateTime() : QDateTime::fromTime_t(r_textDate);
	note->author = r_authorArray;
	note->source = r_sourceArray;
	note->comment = r_commentArray;
	note->iconID = hash.value(r_iconID);
	note->nameBackColor.setRgba(r_backColor);
	note->nameForeColor.setRgba(r_foreColor);
	note->locked = (bool)r_locked;
	foreach(QString name, images.keys()) {
		note->document->addResource(QTextDocument::ImageResource, QUrl(name), images.value(name));
	}
	note->document->setHtml(r_textArray);

	return note;
}

void Note::onChange() {
	modificationDate = QDateTime::currentDateTime();
	emit sg_ModifyDateChanged();
	emit sg_DataChanged();
}

void Note::sl_TagsCollectionModified(Tag*) {
	onChange();
}