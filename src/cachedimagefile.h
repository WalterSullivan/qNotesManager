#ifndef CACHEDIMAGEFILE_H
#define CACHEDIMAGEFILE_H

#include "cachedfile.h"

#include <QSize>
#include <QImage>
#include <QPixmap>

namespace qNotesManager {
	class CachedImageFile : public CachedFile {
	public:
		CachedImageFile(const QByteArray& array, QString name, QString format);

		QString Format;

		bool IsValidImage() const;
		QSize ImageSize() const;
		QImage GetImage() const;
		QPixmap GetPixmap(QSize preferredSize) const;

		static CachedImageFile* FromFile(QString fileName);
	};
}
#endif // CACHEDIMAGEFILE_H
