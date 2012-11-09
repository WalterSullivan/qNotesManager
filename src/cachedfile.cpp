#include "cachedfile.h"


using namespace qNotesManager;

CachedFile::CachedFile(const QByteArray& array, QString name) :
	Data(array),
	FileName(name) {
}
