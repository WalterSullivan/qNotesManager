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

#include "imageloader.h"

#include "cachedimagefile.h"
#include "httpimagedownloader.h"
#include "localimageloader.h"

using namespace qNotesManager;

ImageLoader::ImageLoader(QObject *parent) : QObject(parent) {
	localImageLoader = new LocalImageLoader(this);
	httpImageLoader = new HttpImageDownloader(this);

	QObject::connect(localImageLoader, SIGNAL(sg_DownloadError(QUrl,QString)),
					 this, SIGNAL(sg_DownloadError(QUrl,QString)));
	QObject::connect(localImageLoader, SIGNAL(sg_DownloadFinished(QUrl,CachedImageFile*)),
					 this, SIGNAL(sg_DownloadFinished(QUrl,CachedImageFile*)));
	QObject::connect(localImageLoader, SIGNAL(sg_Progress(QUrl,int)),
					 this, SIGNAL(sg_Progress(QUrl,int)));

	QObject::connect(httpImageLoader, SIGNAL(sg_DownloadError(QUrl,QString)),
					 this, SIGNAL(sg_DownloadError(QUrl,QString)));
	QObject::connect(httpImageLoader, SIGNAL(sg_DownloadFinished(QUrl,CachedImageFile*)),
					 this, SIGNAL(sg_DownloadFinished(QUrl,CachedImageFile*)));
	QObject::connect(httpImageLoader, SIGNAL(sg_Progress(QUrl,int)),
					 this, SIGNAL(sg_Progress(QUrl,int)));
}

void ImageLoader::Download(const QUrl url) {
	if (url.scheme() == "http" || url.scheme() == "https") {
		httpImageLoader->Download(url);
	} else if (url.scheme() == "file") {
		localImageLoader->Download(url);
	}
}

void ImageLoader::CancelAllDownloads() {
	localImageLoader->CancelAllDownloads();
	httpImageLoader->CancelAllDownloads();
}
