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

#include "boibuffer.h"
#include "global.h"

#include <QBuffer>
#include <QSysInfo>

#define CHECK_FOR_READ if (device == nullptr || !device->isOpen() || !device->isReadable()) {WARNING("Device not ready for reading"); return 0;}

using namespace qNotesManager;

BOIBuffer::BOIBuffer(QIODevice* dev) {
	if (dev == nullptr) {
		WARNING("No device specified");
	}
	device = dev;
	noswap = QSysInfo::ByteOrder == QSysInfo::BigEndian;
}

BOIBuffer::BOIBuffer(QByteArray* array) {
	if (array == nullptr) {
		WARNING("No device specified");
	} else {
		device = new QBuffer(array, this);
	}
	noswap = QSysInfo::ByteOrder == QSysInfo::BigEndian;
}

BOIBuffer::~BOIBuffer() {
	if (device && device->isOpen()) {
		device->close();
	}
}

bool BOIBuffer::open (QIODevice::OpenMode mode) {
	return device->open(mode);
}

void BOIBuffer::close () {
	device->close();
}

qint64 BOIBuffer::pos () const {
	return device->pos();
}

bool BOIBuffer::seek (qint64 pos) {
	return device->seek(pos);
}

qint64 BOIBuffer::size () const {
	return device->size();
}

qint64 BOIBuffer::write(const char* data, qint64 length) {
	return device->write(data, length);
}

qint64 BOIBuffer::write(bool i) {
	return device->write((char*)&i, sizeof(i));
}

qint64 BOIBuffer::write(qint8 i) {
	return device->write((char*)&i, sizeof(i));
}

qint64 BOIBuffer::write(quint8 i) {
	return device->write((char*)&i, sizeof(i));
}

qint64 BOIBuffer::write(qint16 i) {
	if (noswap) {
		return device->write((char*)&i, sizeof(i));
	} else {
		uchar* p = (uchar*)(&i);
		char s[2];

		s[1] = *p++;
		s[0] = *p;

		return device->write(s, sizeof(i));
	}
}

qint64 BOIBuffer::write(quint16 i) {
	if (noswap) {
		return device->write((char*)&i, sizeof(i));
	} else {
		uchar* p = (uchar*)(&i);
		char s[2];

		s[1] = *p++;
		s[0] = *p;

		return device->write(s, sizeof(i));
	}
}

qint64 BOIBuffer::write(qint32 i) {
	if (noswap) {
		return device->write((char*)&i, sizeof(i));
	} else {
		uchar* p = (uchar*)(&i);
		char s[4];

		s[3] = *p++;
		s[2] = *p++;
		s[1] = *p++;
		s[0] = *p;

		return device->write(s, sizeof(i));
	}
}

qint64 BOIBuffer::write(quint32 i) {
	if (noswap) {
		return device->write((char*)&i, sizeof(i));
	} else {
		uchar* p = (uchar*)(&i);
		char s[4];

		s[3] = *p++;
		s[2] = *p++;
		s[1] = *p++;
		s[0] = *p;

		return device->write(s, sizeof(i));
	}
}

qint64 BOIBuffer::write(qint64 i) {
	if (noswap) {
		return device->write((char*)&i, sizeof(i));
	} else {
		uchar* p = (uchar*)(&i);
		char s[8];

		s[7] = *p++;
		s[6] = *p++;
		s[5] = *p++;
		s[4] = *p++;
		s[3] = *p++;
		s[2] = *p++;
		s[1] = *p++;
		s[0] = *p;

		return device->write(s, sizeof(i));
	}
}

qint64 BOIBuffer::write(quint64 i) {
	if (noswap) {
		return device->write((char*)&i, sizeof(i));
	} else {
		uchar* p = (uchar*)(&i);
		char s[8];

		s[7] = *p++;
		s[6] = *p++;
		s[5] = *p++;
		s[4] = *p++;
		s[3] = *p++;
		s[2] = *p++;
		s[1] = *p++;
		s[0] = *p;

		return device->write(s, sizeof(i));
	}
}

qint64 BOIBuffer::write(const QByteArray& array) {
	return device->write(array);
}



qint64 BOIBuffer::read(char* data, qint64 length) {
	CHECK_FOR_READ;
	return device->read(data, length);
}

qint64 BOIBuffer::read(bool& i) {
	CHECK_FOR_READ;
	return device->read((char*)&i, sizeof(i));
}

qint64 BOIBuffer::read(qint8& i) {
	CHECK_FOR_READ;
	return device->read((char*)&i, sizeof(i));
}

qint64 BOIBuffer::read(quint8& i) {
	CHECK_FOR_READ;
	return device->read((char*)&i, sizeof(i));
}

qint64 BOIBuffer::read(qint16& i) {
	CHECK_FOR_READ;
	if (noswap) {
		return device->read((char*)&i, sizeof(i));
	} else {
		uchar* p = (uchar*)(&i);
		char s[2];

		qint64 result = device->read(s, sizeof(i));

		*p++ = s[1];
		*p = s[0];

		return result;
	}
}

qint64 BOIBuffer::read(quint16& i) {
	CHECK_FOR_READ;
	if (noswap) {
		return device->read((char*)&i, sizeof(i));
	} else {
		uchar* p = (uchar*)(&i);
		char s[2];

		qint64 result = device->read(s, sizeof(i));

		*p++ = s[1];
		*p = s[0];

		return result;
	}
}

qint64 BOIBuffer::read(qint32& i) {
	CHECK_FOR_READ;
	if (noswap) {
		return device->read((char*)&i, sizeof(i));
	} else {
		uchar* p = (uchar*)(&i);
		char s[4];

		qint64 result = device->read(s, sizeof(i));

		*p++ = s[3];
		*p++ = s[2];
		*p++ = s[1];
		*p = s[0];

		return result;
	}
}

qint64 BOIBuffer::read(quint32& i) {
	CHECK_FOR_READ;
	if (noswap) {
		return device->read((char*)&i, sizeof(i));
	} else {
		uchar* p = (uchar*)(&i);
		char s[4];

		qint64 result = device->read(s, sizeof(i));

		*p++ = s[3];
		*p++ = s[2];
		*p++ = s[1];
		*p = s[0];

		return result;
	}
}

qint64 BOIBuffer::read(qint64& i) {
	CHECK_FOR_READ;
	if (noswap) {
		return device->read((char*)&i, sizeof(i));
	} else {
		uchar* p = (uchar*)(&i);
		char s[8];

		qint64 result = device->read(s, sizeof(i));

		*p++ = s[7];
		*p++ = s[6];
		*p++ = s[5];
		*p++ = s[4];
		*p++ = s[3];
		*p++ = s[2];
		*p++ = s[1];
		*p = s[0];

		return result;
	}
}

qint64 BOIBuffer::read(quint64& i) {
	CHECK_FOR_READ;
	if (noswap) {
		return device->read((char*)&i, sizeof(i));
	} else {
		uchar* p = (uchar*)(&i);
		char s[8];

		qint64 result = device->read(s, sizeof(i));

		*p++ = s[7];
		*p++ = s[6];
		*p++ = s[5];
		*p++ = s[4];
		*p++ = s[3];
		*p++ = s[2];
		*p++ = s[1];
		*p = s[0];

		return result;
	}
}
