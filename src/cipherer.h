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

#ifndef CIPHERER_H
#define CIPHERER_H

#include <QtCrypto>
#include <QByteArray>
#include <QHash>

namespace qNotesManager {
	class Cipherer {
	private:
		QByteArray process(const QByteArray& data, const QByteArray& keyData,
						   QCA::Direction direction, int cipherID);

		QHash<int, QString> avaliableCipherTypes;
		const QCA::Cipher::Mode defaultCipherMode;
		const QCA::Cipher::Padding defaultCipherPadding;

	public:
		Cipherer();

		QByteArray Encrypt(const QByteArray& data, const QByteArray& keyData, int cipherID);

		QByteArray Decrypt(const QByteArray& data, const QByteArray& keyData, int cipherID);

		QByteArray GetHash(const QByteArray& str, quint8 hashID);
		bool IsHashSupported(quint8);

		QByteArray GetSecureHash(const QByteArray& data, quint8 hashID);
		bool IsSecureHashSupported(quint8);

		QString GetMD5Hash(const QByteArray& data);

		bool IsCipherTypeSupported(QString);

		QString GetCipherName(int cipherID);

		QList<int> GetAvaliableCipherIDs();

		const quint8 DefaultHashID;
		const quint8 DefaultSecureHashID;
	};
}

#endif // CIPHERER_H
