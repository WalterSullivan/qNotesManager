#include "imageloader.h"

#include "cachedimagefile.h"
#include "httpimagedownloader.h"
#include "localimageloader.h"

using namespace qNotesManager;

ImageLoader::ImageLoader(QObject *parent) :
    QObject(parent)
{
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

void ImageLoader::CancelDownload(const QUrl url) {
	if (url.scheme() == "http" || url.scheme() == "https") {
		httpImageLoader->CancelDownload(url);
	} else if (url.scheme() == "file") {
		localImageLoader->CancelDownload(url);
	}
}

void ImageLoader::CancelAllDownloads() {
	localImageLoader->CancelAllDownloads();
	httpImageLoader->CancelAllDownloads();
}
