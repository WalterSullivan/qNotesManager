#include "cachedfile.h"

#include "crc32.h"

#include <QFile>

using namespace qNotesManager;

CachedFile::CachedFile(const QByteArray& array, QString name) :
	Data(array),
	FileName(name) {
}

quint32 CachedFile::GetCRC32() const {
	return crc32buf(Data.constData(), Data.length());
}

// static
CachedFile* CachedFile::FromFile(QString fileName) {
	QFile f(fileName);

	if (!f.exists()) {
		return 0;
	}

	f.open(QIODevice::ReadOnly);
	QByteArray array = f.readAll();
	f.close();

	return new CachedFile(array, fileName);
}
