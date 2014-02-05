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

#include "localimageloader.h"

#include <QTimer>
#include <QFileInfo>
#include <QDebug>


#include "cachedimagefile.h"

using namespace qNotesManager;

void LocalImageLoadWorker::sl_process() {
	while(true) {
		lock.lockForRead();
		if (queue.isEmpty()) {
			lock.unlock();
			break;
		}
		lock.unlock();
		lock.lockForWrite();
		QUrl url = queue.takeFirst();
		lock.unlock();

		QFileInfo info(url.toLocalFile());
		if (!info.exists()) {
			emit sg_DownloadError(url, "File does not exists");
		} else {
			CachedImageFile* image = CachedImageFile::FromFile(info.absoluteFilePath());
			emit sg_DownloadFinished(url, image);
		}
	}

	emit sg_finished();
}

void LocalImageLoadWorker::AddUrl(const QUrl url) {
	lock.lockForWrite();
	queue.append(url);
	lock.unlock();
}

void LocalImageLoadWorker::CancelAllDownloads() {
	lock.lockForWrite();
	queue.clear();
	lock.unlock();
}

LocalImageLoader::LocalImageLoader(QObject *parent) :
	QObject (parent) {
	thread = new QThread(this);
	worker = new LocalImageLoadWorker(0);
	worker->moveToThread(thread);

	QObject::connect(thread, SIGNAL(started()), worker, SLOT(sl_process()));
	QObject::connect(worker, SIGNAL(sg_finished()), thread, SLOT(quit()));

	QObject::connect(worker, SIGNAL(sg_DownloadError(QUrl,QString)),
					 this, SIGNAL(sg_DownloadError(QUrl,QString)), Qt::QueuedConnection);
	QObject::connect(worker, SIGNAL(sg_DownloadFinished(QUrl,CachedImageFile*)),
					 this, SIGNAL(sg_DownloadFinished(QUrl,CachedImageFile*)), Qt::QueuedConnection);
}

LocalImageLoader::~LocalImageLoader() {
	if (thread->isRunning()) {
		worker->CancelAllDownloads();
		thread->wait();
	}
	delete worker;
}

void LocalImageLoader::Download(const QUrl url) {
	worker->AddUrl(url);
	if (!thread->isRunning()) {
		thread->start();
	}
}

void LocalImageLoader::CancelDownload(const QUrl url) {
}

void LocalImageLoader::CancelAllDownloads() {
	worker->CancelAllDownloads();
}
