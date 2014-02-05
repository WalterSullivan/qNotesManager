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
