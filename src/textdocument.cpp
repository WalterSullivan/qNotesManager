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

#include "textdocument.h"

#include "imagedownloader.h"
#include "crc32.h"
#include "global.h"
#include "cachedimagefile.h"

#include <QFileInfo>
#include <QDebug>
#include <QRegExp>
#include <QTextImageFormat>
#include <QTextObject>
#include <QByteArray>
#include <QTextCursor>
#include <QThread>
#include <QPainter>
#include <QBuffer>

using namespace qNotesManager;

TextDocument::TextDocument(QObject *parent) : QTextDocument(parent) {
	downloader = new ImageDownloader(this);

	QObject::connect(downloader, SIGNAL(sg_DownloadError(QUrl,QString)),
					 this, SLOT(sl_Downloader_DownloadError(QUrl,QString)), Qt::DirectConnection);
	QObject::connect(downloader, SIGNAL(sg_DownloadFinished(QUrl,QImage)),
					 this, SLOT(sl_Downloader_DownloadFinished(QUrl,QImage)), Qt::DirectConnection);
	QObject::connect(downloader, SIGNAL(sg_Progress(QUrl,int)),
					 this, SLOT(sl_Downloader_Progress(QUrl,int)), Qt::DirectConnection);

	createDummyImages();

	restartDownloadsTimer.setInterval(60000); // restart in 1 minute
	QObject::connect(&restartDownloadsTimer, SIGNAL(timeout()),
					 this, SLOT(sl_RestartDownloadsTimer_Timeout()));
}

TextDocument::~TextDocument() {
	foreach (const QString name, originalImages.keys()) {
		delete originalImages[name];
	}
	originalImages.clear();
}

void TextDocument::sl_Downloader_DownloadFinished (QUrl url, QImage image) {
	if (errorDownloads.contains(url)) {errorDownloads.removeOne(url);}

	quint32 hash = CalculateImageCRC(image);
	QString name = QString::number(hash);
	QUrl newUrl(name);
	if (!resource(QTextDocument::ImageResource, newUrl).isValid()) {
		qDebug() << "Resource for id " << name << " not found. Adding resource";
		addResource(QTextDocument::ImageResource, newUrl, image);
	} else {
		qDebug() << "Resource for id " << name << " found";
	}

	replaceImageUrl(url.toString(), name);
	// A bug may happen here. QTextEdit sends resource requests anisochronously, and even after
	// 'replaceImageUrl' line a request for old url may come. If such request will come after
	// 'activeDownloads.removeAll(url);' line, old url will be queued for downloading again.

	qDebug() << "Removing " << url << "from active downloads list";
	activeDownloads.removeAll(url);

	emit sg_NeedRelayout();
}

void TextDocument::sl_Downloader_DownloadError (QUrl url, QString errorDescription) {
	if (!errorDownloads.contains(url)) {
		errorDownloads.append(url);
		if (!restartDownloadsTimer.isActive()) {
			restartDownloadsTimer.start();
		}
	}

	qDebug() << "TextDocument: problem loading image: " << url.toString() << " with error: " << errorDescription;
}

void TextDocument::sl_Downloader_Progress(QUrl, int) {}

/* virtual */
QVariant TextDocument::loadResource (int type, const QUrl& url) {
	qDebug() << "\nTextDocument::loadResource called. Requested url : " << url.toString();

	if (!url.isValid()) {
		qDebug() << "Requested resource with invalid url: " << url.toString();
	}

	if (type != QTextDocument::ImageResource) {
		qDebug() << "WARNING: Unknown resource type requested " << type;
		return QVariant();
	}

	if (type == QTextDocument::ImageResource) {
		if (url.scheme() == "http" || url.scheme() == "https") {
			if (errorDownloads.contains(url)) {
				return errorDummyImage;
			}
			if (!activeDownloads.contains(url)/*downloader->HasActiveDownload(name)*/) {
				qDebug() << "Creating download task";
				activeDownloads.append(url);
				downloader->Download(url);
				return QVariant();
			} else {
				qDebug() << "Url " << url << "is in active downloads list. Skipping";
				return loadingDummyImage;
			}
		} else if (url.scheme() == "file") {
			QFileInfo info(url.toLocalFile());
			if (!info.exists()) {
				qDebug() << "Unable to load local file: " << url.toLocalFile();
				return QVariant();
			}
			QImage image(info.absoluteFilePath());
			if (image.isNull()) {
				qDebug() << "Unable to load local file: " << url.toLocalFile();
				return QVariant();
			}
			image.setText("FORMAT", info.suffix());

			quint32 hash = CalculateImageCRC(image);
			QString name = QString::number(hash);
			QUrl newUrl(name);
			if (!resource(QTextDocument::ImageResource, newUrl).isValid()) {
				qDebug() << "Resource for id " << name << " not found. Adding resource";
				addResource(QTextDocument::ImageResource, newUrl, image);
			} else {
				qDebug() << "Resource for id " << name << " found";
			}

			replaceImageUrl(url.toString(), name);

			return image;
		} else {
			qDebug() << "Resource with unknown scheme requested: " << url.toString();
		}
	}
	return QVariant();
}

void TextDocument::createDummyImages() {
	const QSize dummyImageSize = QSize(150, 150);
	loadingDummyImage = QPixmap(dummyImageSize);
	errorDummyImage = QPixmap(dummyImageSize);
	QPainter painter;
	const QString loadingDummyImageText = "Loading image...";
	const QString errorDummyImageText = "Error loading image";
	const QBrush backgroundBrush(Qt::lightGray);
	const QRect rect(QPoint(0,0), dummyImageSize);

	painter.begin(&loadingDummyImage);
	painter.setBrush(backgroundBrush);
	painter.drawRect(rect);
	painter.drawText(rect, Qt::AlignCenter, loadingDummyImageText);
	painter.end();

	painter.begin(&errorDummyImage);
	painter.setBrush(backgroundBrush);
	painter.drawRect(rect);
	painter.drawText(rect, Qt::AlignCenter, errorDummyImageText);
	painter.end();
}

void TextDocument::replaceImageUrl(QString oldName, QString newName) {
	QTextBlock block = begin();
	while(block.isValid()) {
		QTextBlock::iterator iterator;
		for(iterator = block.begin(); !(iterator.atEnd()); ++iterator) {
			QTextFragment fragment = iterator.fragment();
			if(fragment.isValid() && fragment.charFormat().isImageFormat()) {
				QTextImageFormat format = fragment.charFormat().toImageFormat();
				if (format.name() != oldName) {continue;}
				qDebug() << "Changing url from" << format.name() << "to" << newName;
				format.setName(newName);
				QTextCursor cursor(this);
				cursor.setPosition(fragment.position());
				cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, fragment.length());
				cursor.beginEditBlock();
				cursor.setCharFormat(format);
				cursor.endEditBlock();
			}
		}
		block = block.next();
	}
}

QSet<QString> TextDocument::GetImagesList() const {
	QSet<QString> returnSet;

	QTextBlock block = begin();
	while(block.isValid()) {
		QTextBlock::iterator it;
		for(it = block.begin(); !(it.atEnd()); ++it) {
			const QTextFragment currentFragment = it.fragment();
			if(!currentFragment.isValid()) {continue;}
			if(!currentFragment.charFormat().isImageFormat()) {continue;}

			QTextImageFormat format = currentFragment.charFormat().toImageFormat();
			const QByteArray imageName = format.name().toUtf8();
			if (returnSet.contains(QString(imageName))) {continue;}

			QVariant imageData = resource(QTextDocument::ImageResource, QUrl(imageName));
			QImage image = imageData.value<QImage>();
			if (image.cacheKey() == errorDummyImage.cacheKey() ||
				image.cacheKey() == loadingDummyImage.cacheKey()) {
				qDebug() << "Image " << imageName << " not loaded. Skipping.";
				continue;
			}

			returnSet.insert(QString(imageName));

		}
		block = block.next();
	}

	return returnSet;
}

quint32 TextDocument::CalculateImageCRC(const QImage& image) const {
	QByteArray a;
	QBuffer b(&a);
	b.open(QIODevice::WriteOnly);
	image.save(&b, image.text("FORMAT").toStdString().c_str());
	b.close();

	return crc32buf(a.constData(), a.length());

	/*const char* data = a.constData();
	quint32 hash, i;
	quint32 size = a.size();
	for(hash = i = 0; i < size; ++i) {
		hash += data[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;*/
}

void TextDocument::InsertImage(QImage image, QTextCursor cursor) {
	if (image.isNull()) {
		qWarning("Image is null");
		return;
	}
	quint32 crc = CalculateImageCRC(image);
	QUrl url(QString::number(crc));
	if (!resource(QTextDocument::ImageResource, url).isValid()) {
		addResource(QTextDocument::ImageResource, url, image);
	}
	cursor.insertImage(url.toString());
}

void TextDocument::InsertImage(QUrl url, QTextCursor cursor) {
	WARNING("Not implemented");
}

void TextDocument::sl_RestartDownloadsTimer_Timeout() {
	QMutableListIterator<QUrl> it(errorDownloads);

	while (it.hasNext()) {
		QUrl url = it.next();
		activeDownloads.append(url);
		downloader->Download(url);
		it.remove();
	}
}
