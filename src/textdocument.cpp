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

#include "imageloader.h"
#include "crc32.h"
#include "global.h"
#include "cachedimagefile.h"
#include "idummyimagesprovider.h"

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
	loader = new ImageLoader(this);

	QObject::connect(loader, SIGNAL(sg_DownloadError(QUrl,QString)),
					 this, SLOT(sl_Downloader_DownloadError(QUrl,QString)), Qt::DirectConnection);
	QObject::connect(loader, SIGNAL(sg_DownloadFinished(QUrl,CachedImageFile*)),
					 this, SLOT(sl_Downloader_DownloadFinished(QUrl,CachedImageFile*)), Qt::DirectConnection);
	QObject::connect(loader, SIGNAL(sg_Progress(QUrl,int)),
					 this, SLOT(sl_Downloader_Progress(QUrl,int)), Qt::DirectConnection);


	restartDownloadsTimer.setInterval(60000); // restart in 1 minute
	QObject::connect(&restartDownloadsTimer, SIGNAL(timeout()),
					 this, SLOT(sl_RestartDownloadsTimer_Timeout()));

	QFont f;
	f.setFamily("Arial");
	f.setPointSize(9);
	setDefaultFont(f);
}

TextDocument::~TextDocument() {
	foreach (const QString name, originalImages.keys()) {
		delete originalImages[name];
	}
	originalImages.clear();
}

void TextDocument::sl_Downloader_DownloadFinished (QUrl url, CachedImageFile* image) {
	if (errorDownloads.contains(url)) {
		errorDownloads.removeOne(url);
	}

	if (!image || !image->IsValidImage()) {
		if (!errorDownloads.contains(url)) {
			errorDownloads.append(url);
			if (!restartDownloadsTimer.isActive()) {
				restartDownloadsTimer.start();
			}
		}

		if (image) {delete image;}
		activeDownloads.removeAll(url);
		return;
	}

	QString name = image->GetMD5();
	QUrl newUrl(name);
	if (!originalImages.contains(name)) {
		qDebug() << "Resource for id " << name << " not found. Adding resource";
		addResource(QTextDocument::ImageResource, newUrl, image->GetPixmap());
		originalImages.insert(name, image);
	} else {
		qDebug() << "!!!! Resource for id " << name << " found";
		delete image;
	}

	replaceImageUrl(url, name);
	// A bug may happen here. QTextEdit sends resource requests anisochronously, and even after
	// 'replaceImageUrl' line a request for old url may be requested. If such request will come after
	// 'activeDownloads.removeAll(url)' line, old url will be queued for downloading again.

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
		QString stringUrl = url.toString();
		if (originalImages.contains(stringUrl)) {
			CachedImageFile* image = originalImages[stringUrl];
			addResource(QTextDocument::ImageResource, url, image->GetPixmap());
			return image->GetPixmap();
		}
		if (errorDownloads.contains(url)) {
			return DummyImagesProvider->GetErrorImage();
		}
		if (!activeDownloads.contains(url)) {
			qDebug() << "Creating download task";
			activeDownloads.append(url);
			loader->Download(url);
			return DummyImagesProvider->GetLoadingImage();
		} else {
			qDebug() << "Url " << url << "is in active downloads list. Skipping";
			return DummyImagesProvider->GetLoadingImage();
		}
	}

	return QVariant();
}

void TextDocument::replaceImageUrl(const QUrl &oldName, const QString &newName) {
	QList <QPair<int, int> > fragments;

	QTextBlock block = begin();

	while(block.isValid()) {
		QTextBlock::iterator iterator;
		for(iterator = block.begin(); !(iterator.atEnd()); ++iterator) {
			QTextFragment fragment = iterator.fragment();
			if(fragment.isValid() && fragment.charFormat().isImageFormat()) {
				QTextImageFormat format = fragment.charFormat().toImageFormat();
				if (QUrl::fromEncoded(format.name().toUtf8()) != oldName) {continue;}
				fragments.append(QPair<int, int>(fragment.position(), fragment.length()));
			}
		}
		block = block.next();
	}


	QTextCursor cursor(this);
	cursor.beginEditBlock();
	QPair<int, int> pair;
	foreach (pair, fragments) {
		cursor.setPosition(pair.first);
		cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, pair.second);
		QTextImageFormat format = cursor.charFormat().toImageFormat();
		format.setName(newName);
		cursor.mergeCharFormat(format);
	}
	cursor.endEditBlock();
}

QStringList TextDocument::GetImagesList() const {
	QStringList returnSet;

	QTextBlock block = begin();
	while(block.isValid()) {
		QTextBlock::iterator it;
		for(it = block.begin(); !(it.atEnd()); ++it) {
			const QTextFragment currentFragment = it.fragment();
			if(!currentFragment.isValid()) {continue;}
			if(!currentFragment.charFormat().isImageFormat()) {continue;}

			QTextImageFormat format = currentFragment.charFormat().toImageFormat();
			const QString imageName = QString(format.name().toUtf8());
			if (returnSet.contains(imageName)) {continue;}

			if (!originalImages.contains(imageName)) {
				qDebug() << "Image " << imageName << " not loaded. Skipping.";
				continue;
			}

			returnSet.append(imageName);

		}
		block = block.next();
	}

	return returnSet;
}

QStringList TextDocument:: GetResourceImagesList() const {
	return originalImages.keys();
}

void TextDocument::sl_RestartDownloadsTimer_Timeout() {
	QMutableListIterator<QUrl> it(errorDownloads);

	while (it.hasNext()) {
		QUrl url = it.next();
		activeDownloads.append(url);
		loader->Download(url);

		it.remove();
	}
}

CachedImageFile* TextDocument::GetResourceImage(QString name) const {
	return originalImages.contains(name) ? originalImages.value(name) : 0;
}

void TextDocument::AddResourceImage(CachedImageFile* image) {
	QString md5 = image->GetMD5();
	originalImages.insert(md5, image);
}
