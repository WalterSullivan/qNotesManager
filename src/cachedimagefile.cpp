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

#include "cachedimagefile.h"

#include <QFile>
#include <QFileInfo>

using namespace qNotesManager;

CachedImageFile::CachedImageFile(const QByteArray& array, const QString& name, const QString& format) :
		CachedFile(array, name),
		cachedPixmapSize(QSize()),
		cachedPixmap(0),
		cachePixmapInitialized(false),
		Format(format) {

}

CachedImageFile::~CachedImageFile() {
	if (cachedPixmap) {
		delete cachedPixmap;
	}
}

void CachedImageFile::initCachePixmap() const {
	if (cachePixmapInitialized) {return;}

	cachedPixmap = new QPixmap();
	cachedPixmap->loadFromData(Data, Format.toStdString().c_str());
	cachedPixmapSize = cachedPixmap->size();

	cachePixmapInitialized = true;
}

bool CachedImageFile::IsValidImage() const {
	if (!cachePixmapInitialized) {initCachePixmap();}

	return !cachedPixmap->isNull();
}

QSize CachedImageFile::ImageSize() const {
	if (!cachePixmapInitialized) {initCachePixmap();}
	return cachedPixmap->isNull() ? QSize() : cachedPixmap->size();
}

QPixmap CachedImageFile::GetPixmap(const QSize& preferredSize) const {
	if (!cachePixmapInitialized) {initCachePixmap();}

	if (preferredSize.isValid() && preferredSize != cachedPixmapSize) {
		*cachedPixmap = cachedPixmap->scaled(preferredSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
		cachedPixmapSize = preferredSize;
	}

	return *cachedPixmap;
}

CachedImageFile* CachedImageFile::FromFile(const QString& fileName) {
	QFile file(fileName);

	if (!file.exists()) {
		return 0;
	}

	if (!file.open(QIODevice::ReadOnly)) {return 0;}

	QByteArray array = file.readAll();
	file.close();

	return new CachedImageFile(array, QFileInfo(fileName).fileName(), QFileInfo(fileName).suffix());
}
