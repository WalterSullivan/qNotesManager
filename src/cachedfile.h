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

#ifndef CACHEDFILE_H
#define CACHEDFILE_H

#include <QString>
#include <QByteArray>

namespace qNotesManager {
	class CachedFile {
	private:
		mutable quint32 cachedCrc32;

	protected:
		QByteArray Data;
		QString FileName;

	public:
		explicit CachedFile(const QByteArray& array, const QString& name);
		virtual ~CachedFile() {}

		quint32 GetCRC32() const;
		int Size() const;

		const char* GetData() const;
		QString GetFileName() const;

		bool HasSameDataAs(const CachedFile* other) const;

		bool Save(const QString& fileName) const;

		static CachedFile* FromFile(const QString& fileName);
	};
}


#endif // CACHEDFILE_H
