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
#include <QFileInfo>
#include <QTemporaryFile>
#include <QCryptographicHash>

using namespace qNotesManager;

CachedFile::CachedFile(const QByteArray& array, const QString& name) :
	cachedCrc32(0),
	cachedMD5(QString()),
	Data(array),
	FileName(name) {
}

quint32 CachedFile::GetCRC32() const {
	if (cachedCrc32 == 0) {
		cachedCrc32 = crc32buf(Data.constData(), Data.length());
	}
	return cachedCrc32;
}

QString CachedFile::GetMD5() const {
	if (cachedMD5.isEmpty()) {
		QCryptographicHash hash(QCryptographicHash::Md5);
		hash.addData(Data);
		cachedMD5 = QString(hash.result().toHex());
	}
	return cachedMD5;
}

int CachedFile::Size() const {
	return Data.size();
}

const char* CachedFile::GetData() const {
	return Data.constData();
}

QString CachedFile::GetFileName() const {
	return FileName;
}

bool CachedFile::HasSameDataAs(const CachedFile* other) const {
	return Data == other->Data;
}

bool CachedFile::Save(const QString& fileName) const {
	if (fileName.isEmpty()) {
		return false;
	}

	if (Data.size() == 0) {return false;}

	QFile file(fileName);

	if (!file.open(QIODevice::WriteOnly)) {return false;}
	qint64 result = file.write(Data);
	file.close();

	if (result != Data.size()) {return false;}

	return true;
}

QString CachedFile::SaveToTempFolder() const {
	if (Data.size() == 0) {return QString();}

	QTemporaryFile file;
	file.setAutoRemove(false);

	if (!file.open()) {return QString();}
	qint64 result = file.write(Data);
	file.close();

	if (result != Data.size()) {return QString();}

	return file.fileName();
}

// static
CachedFile* CachedFile::FromFile(const QString& fileName) {
	QFile file(fileName);

	if (!file.exists()) {
		return nullptr;
	}

	if (!file.open(QIODevice::ReadOnly)) {return nullptr;}

	QByteArray array = file.readAll();
	file.close();

	QFileInfo fileInfo(file);

	return new CachedFile(array, fileInfo.fileName());
}
