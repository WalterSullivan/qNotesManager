#include "cachedfile.h"

#include "crc32.h"

using namespace qNotesManager;

CachedFile::CachedFile(const QByteArray& array, QString name) :
	Data(array),
	FileName(name) {
}

quint32 CachedFile::GetCRC32() const {
	return crc32buf(Data.constData(), Data.length());
}
