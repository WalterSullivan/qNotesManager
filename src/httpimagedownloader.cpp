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

#include "httpimagedownloader.h"

#include "global.h"
#include "cachedimagefile.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QNetworkProxy>


using namespace qNotesManager;

HttpImageDownloader::HttpImageDownloader(QObject *parent) :
		QObject(parent),
		OriginalUrlAttribute((QNetworkRequest::Attribute)1000) {
	manager = new QNetworkAccessManager(this);

	connect(manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
			this, SLOT(sl_netManager_authenticationRequired(QNetworkReply*,QAuthenticator*)), Qt::DirectConnection);
	connect(manager, SIGNAL(finished(QNetworkReply*)),
			this, SLOT(sl_netManager_finished(QNetworkReply*)), Qt::DirectConnection);
	connect(manager, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)),
			this, SLOT(sl_netManager_proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)), Qt::DirectConnection);
	connect(manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
			this, SLOT(sl_netManager_sslErrors(QNetworkReply*,QList<QSslError>)), Qt::DirectConnection);

	networkErrorMessages.insert(0,		"QNetworkReply::NoError");
	networkErrorMessages.insert(1,		"QNetworkReply::ConnectionRefusedError");
	networkErrorMessages.insert(2,		"QNetworkReply::RemoteHostClosedError");
	networkErrorMessages.insert(3,		"QNetworkReply::HostNotFoundError");
	networkErrorMessages.insert(4,		"QNetworkReply::TimeoutError");
	networkErrorMessages.insert(5,		"QNetworkReply::OperationCanceledError");
	networkErrorMessages.insert(6,		"QNetworkReply::SslHandshakeFailedError");
	networkErrorMessages.insert(101,	"QNetworkReply::ProxyConnectionRefusedError");
	networkErrorMessages.insert(102,	"QNetworkReply::ProxyConnectionClosedError");
	networkErrorMessages.insert(103,	"QNetworkReply::ProxyNotFoundError");
	networkErrorMessages.insert(104,	"QNetworkReply::ProxyTimeoutError");
	networkErrorMessages.insert(105,	"QNetworkReply::ProxyAuthenticationRequiredError");
	networkErrorMessages.insert(201,	"QNetworkReply::ContentAccessDenied");
	networkErrorMessages.insert(202,	"QNetworkReply::ContentOperationNotPermittedError");
	networkErrorMessages.insert(203,	"QNetworkReply::ContentNotFoundError");
	networkErrorMessages.insert(204,	"QNetworkReply::AuthenticationRequiredError");
	networkErrorMessages.insert(205,	"QNetworkReply::ContentReSendError");
	networkErrorMessages.insert(301,	"QNetworkReply::ProtocolUnknownError");
	networkErrorMessages.insert(302,	"QNetworkReply::ProtocolInvalidOperationError");
	networkErrorMessages.insert(99,		"QNetworkReply::UnknownNetworkError");
	networkErrorMessages.insert(199,	"QNetworkReply::UnknownProxyError");
	networkErrorMessages.insert(299,	"QNetworkReply::UnknownContentError");
	networkErrorMessages.insert(399,	"QNetworkReply::ProtocolFailure");
}

HttpImageDownloader::~HttpImageDownloader() {
	CancelAllDownloads();
}

void HttpImageDownloader::SetProxy(QNetworkProxy& p) {
	manager->setProxy(p);
}

bool HttpImageDownloader::IsProxyEnabled() const {
	return manager->proxy().type() != QNetworkProxy::NoProxy;
}

void HttpImageDownloader::DisableProxy() {
	manager->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
}

void HttpImageDownloader::Download(QUrl url) {
	if (activeDownloads.contains(url)) {return;}

	QNetworkReply* reply = manager->get(QNetworkRequest(url));

	activeDownloads.insert(url, reply);

	connect(reply, SIGNAL(downloadProgress(qint64,qint64)),this, SLOT(sl_reply_downloadProgress(qint64,qint64)));
}

void HttpImageDownloader::CancelDownload(QUrl url) {
	if (activeDownloads.contains(url)) {
		activeDownloads[url]->abort();
		activeDownloads.remove(url);
	}
}

void HttpImageDownloader::CancelAllDownloads() {
	foreach (QUrl url, activeDownloads.keys()) {
		activeDownloads[url]->abort();
		activeDownloads.remove(url);
	}
}

QList<QUrl> HttpImageDownloader::ActiveDownloads() const {
	return activeDownloads.keys();
}

bool HttpImageDownloader::HasActiveDownload(const QUrl url) const {
	return activeDownloads.contains(url);
}

void HttpImageDownloader::sl_reply_downloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
	QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
	if (reply == nullptr) {
		WARNING("Casting error");
		return;
	}

	QUrl url = extractOriginalUrl(reply);
	qint64 progress = 0;

	if (bytesTotal != 0 && bytesReceived != 0){
		progress = (bytesReceived * 100) / bytesTotal;
	}
	emit sg_Progress(url, (int)progress);
}

QUrl HttpImageDownloader::extractOriginalUrl(QNetworkReply* reply) {
	if (reply == nullptr) {
		WARNING("Null pointer recieved");
		return QUrl();
	}

	QUrl originalRequestUrl = reply->request().attribute(OriginalUrlAttribute).isValid() ?
				reply->request().attribute(OriginalUrlAttribute).toUrl() :
				reply->request().url();

	return originalRequestUrl;
}

void HttpImageDownloader::sl_netManager_finished (QNetworkReply * reply) {
	if (reply == nullptr) {
		WARNING("Null pointer recieved");
		return;
	}

	QUrl currentReplyUrl = reply->url();
	QUrl originalRequestUrl = extractOriginalUrl(reply);

	// Pring reply info
	qDebug() << "\nDownload finished";
	qDebug() << "Reply size: " << reply->size();
	qDebug() << "Reply url: " << reply->url().toString();
	qDebug() << "Reply isFinished: " << reply->isFinished() << "\n";

	QNetworkReply::NetworkError error = reply->error();
	if (error != QNetworkReply::NoError) {
		qDebug() << "ERROR: Cannot load file: " << reply->request().url().toString();
		downloadFailed(originalRequestUrl, "Cannot download file. Error type: " + networkErrorMessages[error]);
		disconnectAndDeleteReply(reply);
		return;
	} else {
		qDebug() << "Reply: No errors";
	}

	QVariant atr = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
	if (atr.isValid() && !atr.toString().isEmpty()) {
		qDebug() << "Redirect to: " << atr.toString();

		QNetworkRequest request = QNetworkRequest(atr.toUrl());
		// In case of redirect put original url into OriginalUrlAttribute of new request
		request.setAttribute(OriginalUrlAttribute, originalRequestUrl);

		QNetworkReply* newReply = manager->get(request);
		activeDownloads[originalRequestUrl] = newReply;
		connect(newReply, SIGNAL(downloadProgress(qint64,qint64)),this, SLOT(sl_reply_downloadProgress(qint64,qint64)));
		disconnectAndDeleteReply(reply);
		return;
	} else {
		qDebug() << "No redirect";
	}



	qDebug() << "currentReplyUrl: " << currentReplyUrl.toString();
	qDebug() << "originalRequestUrl: " << originalRequestUrl.toString();

	// Detect image type
	QByteArray fileSuffix;
	{
		// Try to read image type from reply's ContentTypeHeader
		if (fileSuffix.isEmpty()) {
			QVariant v = reply->header(QNetworkRequest::ContentTypeHeader);
			if (v.isValid()) {
				QString data = v.toString().toLower();
				QString s1 = data.mid(0, 6);
				QString s2 = data.mid(6, data.length() - 6);

				if (s1.isNull() || s1.isEmpty() || s1 != "image/") {
					downloadFailed(originalRequestUrl, "Downloaded data is not an image. Header:" + v.toString());
#ifdef DEBUG
					// To check what was returned
					QFile file("lastFile");
					file.open(QIODevice::WriteOnly | QIODevice::Truncate);
					file.write(reply->readAll());
					file.close();
					qDebug() << "Data saved in file 'lastFile' for inspection";
#endif

					disconnectAndDeleteReply(reply);
					return;
				}

				if (!s1.isNull() && !s1.isEmpty() && s1 == "image/" && !s2.isNull() && !s2.isEmpty()) {
					fileSuffix = s2.toLatin1();
					qDebug() << "Filetype detected from reply content type header";
				}
			}
		}

		// Try to detect image type from url:
		if (fileSuffix.isEmpty()) {
			QFileInfo info(currentReplyUrl.toString());
			if (!info.suffix().isEmpty()) {
				fileSuffix = info.suffix().toLower().toLatin1();
				qDebug() << "Filetype detected from reply URL";
			}
		}

		// Otherwise try to read it from reply's location header
		if (fileSuffix.isEmpty()) {
			QVariant v = reply->header(QNetworkRequest::LocationHeader);
			if (v.isValid()) {
				QFileInfo info = QFileInfo(v.toString());
				if (!info.suffix().isEmpty()) {
					fileSuffix = info.suffix().toLower().toLatin1();
					qDebug() << "Filetype detected from reply location header";
				}
			}
		}
		qDebug() << "Recognized file suffix: " << fileSuffix;
	}

	if ((fileSuffix.isEmpty()) || (!QImageReader::supportedImageFormats().contains(fileSuffix))) {
		// If suffix is empty or suffix is not empty, but Qt doesn't support this file type
		qDebug() << "ERROR: Unknown or unsupported file type";
#ifdef DEBUG
			// To check what was returned
			QFile file("lastFile");
			file.open(QIODevice::WriteOnly | QIODevice::Truncate);
			file.write(reply->readAll());
			file.close();
			qDebug() << "Data saved in file 'lastFile' for inspection";
#endif
		downloadFailed(originalRequestUrl, "Unknown or unsupported file type");
		disconnectAndDeleteReply(reply);
		return;
	}

	QByteArray replyData = reply->readAll();
	QFileInfo info(currentReplyUrl.toString());
	CachedImageFile* image = new CachedImageFile(replyData, info.fileName(), fileSuffix);

	activeDownloads.remove(originalRequestUrl);
	emit sg_DownloadFinished(originalRequestUrl, image);
	disconnectAndDeleteReply(reply);
}

void HttpImageDownloader::disconnectAndDeleteReply(QNetworkReply* reply) {
	if (reply == nullptr) {
		WARNING("Null pointer recieved");
		return;
	}

	reply->disconnect();
	reply->deleteLater();
}

void HttpImageDownloader::downloadFailed(QUrl url, QString message) {
	activeDownloads.remove(url);
	emit sg_DownloadError(url, message);
}

void HttpImageDownloader::downloadSucceded(QUrl) {

}

void HttpImageDownloader::sl_netManager_authenticationRequired (QNetworkReply*, QAuthenticator*) {
	WARNING("sl_netManager_authenticationRequired");
}

void HttpImageDownloader::sl_netManager_proxyAuthenticationRequired (const QNetworkProxy&, QAuthenticator*) {
	WARNING("sl_netManager_proxyAuthenticationRequired");
}

void HttpImageDownloader::sl_netManager_sslErrors (QNetworkReply*, const QList<QSslError>&) {
	WARNING("sl_netManager_sslError");
}
