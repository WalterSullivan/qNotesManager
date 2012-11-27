#include "cachedimagefile.h"

#include <QFile>
#include <QFileInfo>

using namespace qNotesManager;

CachedImageFile::CachedImageFile(const QByteArray& array, QString name, QString format) :
		CachedFile(array, name),
		cachedPixmapSize(QSize()),
		cachedPixmap(QPixmap()),
		Format(format) {
	cachedPixmap.loadFromData(Data, Format.toStdString().c_str());
	cachedPixmapSize = cachedPixmap.size();
}

bool CachedImageFile::IsValidImage() const {
	return !cachedPixmap.isNull();
}

QSize CachedImageFile::ImageSize() const {
	return cachedPixmap.isNull() ? QSize() : cachedPixmap.size();
}

QPixmap CachedImageFile::GetPixmap(QSize preferredSize) const {
	if (cachedPixmap.isNull()) {
		return cachedPixmap;
	}

	if (preferredSize.isValid() && preferredSize != cachedPixmapSize) {
		pixmap = pixmap.scaled(preferredSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
		cachedPixmapSize = preferredSize;
	}

	return cachedPixmap;
}

CachedImageFile* CachedImageFile::FromFile(QString fileName) {
	QFile f(fileName);

	if (!f.exists()) {
		return 0;
	}

	f.open(QIODevice::ReadOnly);
	QByteArray array = f.readAll();
	f.close();

	return new CachedImageFile(array, fileName, QFileInfo(fileName).suffix());
}
