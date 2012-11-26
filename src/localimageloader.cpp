#include "localimageloader.h"

#include <QTimer>
#include <QFileInfo>
#include <QDebug>

#include "cachedimagefile.h"

using namespace qNotesManager;

LocalImageLoader::LocalImageLoader(QObject *parent) :
	QObject (parent) {
}

void LocalImageLoader::Download(const QUrl url) {
	queue.append(url);
	QTimer::singleShot(0, this, SLOT(sl_processDownloads()));
}

void LocalImageLoader::CancelDownload(const QUrl url) {
	if (queue.contains(url)) {
		queue.removeAll(url);
	}
}

void LocalImageLoader::CancelAllDownloads() {
	queue.clear();
}

void LocalImageLoader::sl_processDownloads() {
	foreach (QUrl url, queue) {
		QFileInfo info(url.toLocalFile());
		if (!info.exists()) {
			qDebug() << "Unable to load local file: " << url.toLocalFile();
			emit sg_DownloadError(url, "File does not exists");
		} else {
			CachedImageFile* image = CachedImageFile::FromFile(info.absoluteFilePath());
			emit sg_DownloadFinished(url, image);
		}

		queue.removeAll(url);
	}
}
