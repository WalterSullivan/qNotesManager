#include "cachedimagefile.h"

using namespace qNotesManager;

CachedImageFile::CachedImageFile(const QByteArray& array, QString name, QString format) :
		CachedFile(array, name),
		Format(format) {
}

bool CachedImageFile::IsValidImage() const {
	return !QImage::fromData(Data, Format.toStdString().c_str()).isNull();
}

QSize CachedImageFile::ImageSize() const {
	QImage image = QImage::fromData(Data, Format.toStdString().c_str());
	return image.isNull() ? QSize() : image.size();
}

QImage CachedImageFile::GetImage() const {
	return QImage::fromData(Data, Format.toStdString().c_str());
}

QPixmap CachedImageFile::GetPixmap(QSize preferredSize) const {
	QPixmap pixmap;
	if (!pixmap.loadFromData(Data, Format.toStdString().c_str())) {
		return QPixmap();
	}
	if (preferredSize.isValid()) {
		pixmap = pixmap.scaled(preferredSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
	}

	return pixmap;
}
