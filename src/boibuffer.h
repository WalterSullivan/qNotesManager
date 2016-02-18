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

#ifndef BOIBUFFER_H
#define BOIBUFFER_H

#include <QObject>
#include <QIODevice>
#include <QByteArray>

namespace qNotesManager {
	class BOIBuffer : public QObject {
	Q_OBJECT
	public:
		explicit BOIBuffer(QIODevice* dev);
		explicit BOIBuffer(QByteArray* array);

		~BOIBuffer();

		bool open (QIODevice::OpenMode mode);
		void close ();

		qint64 pos () const;
		bool seek (qint64 pos);

		qint64 size () const;

		qint64 write(const char* data, qint64 length);

		qint64 write(bool i);

		qint64 write(qint8 i);
		qint64 write(quint8 i);

		qint64 write(qint16 i);
		qint64 write(quint16 i);

		qint64 write(qint32 i);
		qint64 write(quint32 i);

		qint64 write(qint64 i);
		qint64 write(quint64 i);

		qint64 write(const QByteArray&);




		qint64 read(char* data, qint64 length);

		qint64 read(bool& i);

		qint64 read(qint8& i);
		qint64 read(quint8& i);

		qint64 read(qint16& i);
		qint64 read(quint16& i);

		qint64 read(qint32& i);
		qint64 read(quint32& i);

		qint64 read(qint64& i);
		qint64 read(quint64& i);

	private:
		QIODevice* device;
		bool noswap;

		BOIBuffer(const BOIBuffer&) = delete;
		BOIBuffer& operator=(const BOIBuffer&) = delete;

	};
}

#endif // BOIBUFFER_H
