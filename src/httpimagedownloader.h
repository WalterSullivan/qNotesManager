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

#ifndef HTTPIMAGEDOWNLOADER_H
#define HTTPIMAGEDOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>
#include <QMap>
#include <QImage>


namespace qNotesManager {
	class CachedImageFile;

	class HttpImageDownloader : public QObject {
	Q_OBJECT
	public:
		explicit HttpImageDownloader(QObject *parent);
		~HttpImageDownloader();

		void SetProxy(QNetworkProxy&);
		bool IsProxyEnabled() const;
		void DisableProxy();

		/* virtual */ void Download(const QUrl);
		/* virtual */ void CancelDownload(const QUrl);
		/* virtual */ void CancelAllDownloads();

		QList<QUrl> ActiveDownloads() const;
		bool HasActiveDownload(const QUrl) const;

	private:
		const QNetworkRequest::Attribute OriginalUrlAttribute;

		QNetworkAccessManager*		manager;
		QMap<int, QString>			networkErrorMessages;
		QMap<QUrl, QNetworkReply*>	activeDownloads;

		void downloadFailed(QUrl url, QString message);
		void downloadSucceded(QUrl);
		QUrl extractOriginalUrl(QNetworkReply*);
		void disconnectAndDeleteReply(QNetworkReply*);

	signals:
		void sg_DownloadFinished (QUrl url, CachedImageFile* image);
		void sg_DownloadError (QUrl url, QString errorDescription);
		void sg_Progress(QUrl url, int percent);

	private slots:
		void sl_netManager_authenticationRequired (QNetworkReply* reply, QAuthenticator* authenticator);
		void sl_netManager_finished (QNetworkReply* reply);
		void sl_netManager_proxyAuthenticationRequired (const QNetworkProxy& proxy, QAuthenticator* authenticator);
		void sl_netManager_sslErrors (QNetworkReply* reply, const QList<QSslError>& errors);

		void sl_reply_downloadProgress(qint64,qint64);

	};
}

#endif // HTTPIMAGEDOWNLOADER_H
