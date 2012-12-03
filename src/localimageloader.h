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

#ifndef LOCALIMAGELOADER_H
#define LOCALIMAGELOADER_H

#include <QObject>
#include <QList>
#include <QUrl>
#include <QReadWriteLock>
#include <QThread>

namespace qNotesManager {
	class CachedImageFile;

	class LocalImageLoadWorker : public QObject {
		Q_OBJECT
	private:
		QList<QUrl> queue;
		QReadWriteLock lock;

	public:
		explicit LocalImageLoadWorker(QObject* parent) {};
		void AddUrl(const QUrl);
		void CancelAllDownloads();

	signals:
		void sg_DownloadFinished (QUrl url, CachedImageFile* image);
		void sg_DownloadError (QUrl url, QString errorDescription);

		void sg_finished();

	public slots:
		void sl_process();
	};

	class LocalImageLoader : public QObject {
	Q_OBJECT
	private:
		QList<QUrl> queue;
		QThread* thread;
		LocalImageLoadWorker* worker;

	public:
		explicit LocalImageLoader(QObject *parent);
		~LocalImageLoader();

		void Download(const QUrl);
		void CancelDownload(const QUrl);
		void CancelAllDownloads();

	signals:
		void sg_DownloadFinished (QUrl url, CachedImageFile* image);
		void sg_DownloadError (QUrl url, QString errorDescription);
		void sg_Progress(QUrl url, int percent);
	};
}


#endif // LOCALIMAGELOADER_H
