#ifndef CACHEDFILE_H
#define CACHEDFILE_H

#include <QString>
#include <QByteArray>

namespace qNotesManager {
	class BOIBuffer;

	class CachedFile {
	public:
		explicit CachedFile(const QByteArray& array, QString name);

		QByteArray Data;
		QString FileName;

		quint32 GetCRC32() const;


		//void Serialize(const int version, BOIBuffer& stream);

	};
}


#endif // CACHEDFILE_H
