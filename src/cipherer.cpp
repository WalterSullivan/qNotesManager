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

#include "cipherer.h"

#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <QDebug>

using namespace qNotesManager;

Cipherer::Cipherer() :
		DefaultHashID(0),
		DefaultSecureHashID(0)

{
	avaliableCipherTypes.insert(1, "aes128");
}

QByteArray Cipherer::Encrypt(const QByteArray& data, const QByteArray& keyData, int cipherID) {
	return process(data, keyData, Direction::Encode, cipherID);
}

QByteArray Cipherer::Decrypt(const QByteArray& data, const QByteArray& keyData, int cipherID) {
	return process(data, keyData, Direction::Decode, cipherID);
}

QByteArray Cipherer::process(const QByteArray& data, const QByteArray& keyData,
							Direction direction, int cipherID) {

	if (data.isEmpty()) {
		return QByteArray();
	}
	if (keyData.isEmpty()) {
		return QByteArray();
	}
	if (!avaliableCipherTypes.contains(cipherID)) {
		return QByteArray();
	}

	QByteArray initVectorData ("aes128-cbc-pkcs7", 16);

	QByteArray formalizedKey = keyData.leftJustified(16, '\0', true);
	unsigned char aesKey[16];
	memset(aesKey, 0, 16);
	memcpy(aesKey, formalizedKey.data(), 16);

	const unsigned char* inputData = (const unsigned char*)data.data();
	size_t inputLength = data.length();


	QByteArray resultArray;

	EVP_CIPHER_CTX *ctx;

	if (direction == Direction::Encode) {
		const size_t approximateEncodedDataLength = ((inputLength + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
		unsigned char* encodedData = new unsigned char[approximateEncodedDataLength];
		memset(encodedData, 0, approximateEncodedDataLength);


		int len;
		int actualEncodedDataLength;

		/* Create and initialise the context */
		if(!(ctx = EVP_CIPHER_CTX_new())) {
			delete[] encodedData;
			return QByteArray();
		}

		if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, aesKey, (uchar*)initVectorData.data())) {
			delete[] encodedData;
			return QByteArray();
		}

		if(1 != EVP_EncryptUpdate(ctx, encodedData, &len, inputData, inputLength)) {
			delete[] encodedData;
			return QByteArray();
		}
		actualEncodedDataLength = len;
    
		if(1 != EVP_EncryptFinal_ex(ctx, encodedData + len, &len)) {
			delete[] encodedData;
			return QByteArray();
		}
		actualEncodedDataLength += len;

		EVP_CIPHER_CTX_free(ctx);

		resultArray = QByteArray((const char*)encodedData, actualEncodedDataLength);
		delete[] encodedData;

	} else {

		unsigned char* decodedData = new unsigned char[inputLength];
		memset(decodedData, 0, inputLength);

		int len;
		int actualDecodedDataLength;

		if(!(ctx = EVP_CIPHER_CTX_new())) {
			delete[] decodedData;
			 return QByteArray();
		}

		if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, aesKey, (uchar*)initVectorData.data())) {
			delete[] decodedData;
			return QByteArray();
		}

		if(1 != EVP_DecryptUpdate(ctx, decodedData, &len, inputData, inputLength)) {
			delete[] decodedData;
			return QByteArray();
		}
		actualDecodedDataLength = len;

		if(1 != EVP_DecryptFinal_ex(ctx, decodedData + len, &len)) {
			delete[] decodedData;
			return QByteArray();
		}
		actualDecodedDataLength += len;

		EVP_CIPHER_CTX_free(ctx);

		resultArray = QByteArray((const char*)decodedData, actualDecodedDataLength);
		delete[] decodedData;
	}


	return resultArray;
}

QList<int> Cipherer::GetAvaliableCipherIDs() {
	return avaliableCipherTypes.keys();
}

QString Cipherer::GetCipherName(int cipherID) {
	if (!avaliableCipherTypes.contains(cipherID)) {
		return QString();
	}
	return avaliableCipherTypes.value(cipherID);
}

QByteArray Cipherer::GetHash(const QByteArray& str, quint8 hashID) {
	(void)hashID;
	const int iterations {1000};

	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);

	for (int i = 0; i < iterations; i++) {
		SHA256_Update(&sha256, str.data(), str.size());
	}
	SHA256_Final(hash, &sha256);

	QByteArray finalHash {(char*)hash};

	return finalHash;
}

bool Cipherer::IsHashSupported(quint8 i) {
	return (i == 0);
}

QByteArray Cipherer::GetSecureHash(const QByteArray& data, quint8 hashID) {
	(void)hashID;
	const int iterations {1000};

	unsigned int len {SHA256_DIGEST_LENGTH};
	unsigned char hash[SHA256_DIGEST_LENGTH];

	HMAC_CTX *ctx = HMAC_CTX_new();
	HMAC_Init_ex(ctx, data.data(), data.size(), EVP_sha256(), NULL);
	for (int i = 0; i < iterations; i++) {
		HMAC_Update(ctx, (const uchar*)data.data(), data.size());
	}
	HMAC_Final(ctx, hash, &len);
    HMAC_CTX_free(ctx);

	QByteArray finalHash {(char*)hash};

	return finalHash;
}

bool Cipherer::IsSecureHashSupported(quint8 i) {
	return (i == 0); // FIXME
}
