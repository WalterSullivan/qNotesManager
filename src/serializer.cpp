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

#include "serializer.h"

#include "global.h"
#include "crc32.h"
#include "cipherer.h"
#include "compressor.h"
#include "note.h"
#include "tag.h"
#include "folder.h"
#include "cachedimagefile.h"

#include <QFile>
#include <QFileInfo>
#include <QApplication>
#include <QStack>
#include <QDebug>

using namespace qNotesManager;

Serializer::Serializer() : QObject(0) {
	doc = 0;
	operation = Unknown;
	filename = QString();
}

void Serializer::Load(Document* d, QString _filename) {
	doc = d;
	filename = _filename;
	operation = Loading;
}

void Serializer::Save(Document* d, QString _filename, quint16 _version) {
	doc = d;
	filename = _filename;
	operation = Saving;
	version = _version;
}

void Serializer::sl_start() {
	if (!doc || operation == Unknown || filename.isEmpty()) {
		WARNING("Wrong argument");
		emit sg_finished();
		return;
	}

	switch (operation) {
	case Loading:
		load();
		break;
	case Saving:
		save();
		break;
	case Unknown:
	default:
		WARNING("Wrong case branch");
		break;

	}

	emit sg_finished();
}

void Serializer::load() {
	emit sg_LoadingStarted();

	QFile file(filename);
	if (!file.exists()) {
		emit sg_LoadingFailed("File not found. Make sure it exists and you have read permissions");
		return;
	}

	if (file.size() < 13) {
		// must me guaranteed program can read 9 bytes of signature and file CRC
		emit sg_LoadingFailed("File is too short. Either file is corrupted or it is not a qNotesManager save file");
		return;
	}

	if (!file.open(QIODevice::ReadOnly)) {
		emit sg_LoadingFailed("Could not read the file. Make sure it exists and you have read permissions");
		return;
	}

	qint64 readResult = 0;

	QByteArray fileDataArray(file.size(), 0x0);
	readResult = file.read(fileDataArray.data(), fileDataArray.size());
	file.close();

	BOIBuffer buffer(&fileDataArray);
	buffer.open(QIODevice::ReadOnly);

	const char s[9] = {0x89, 0x51, 0x4E, 0x4D, 0x53, 0x0D, 0x0A, 0x1A, 0x0A};
	const QByteArray standardFileSignature(s, 9);
	QByteArray signature(9, 0x0);
	readResult = buffer.read(signature.data(), 9);
	if (signature != standardFileSignature) {
		emit sg_LoadingFailed("Selected file is not a qNotesManager save file");
		return;
	}

	{ // Check CRC
		quint64 currentpos = buffer.pos();
		buffer.seek(fileDataArray.size() - 4);
		quint32 crc = 0;
		buffer.read(crc);

		quint32 actualCrc = crc32buf(fileDataArray.constData(), fileDataArray.size() - 4);
		if (crc != actualCrc) {
			emit sg_LoadingFailed("File data corrupted");
			return;
		}

		buffer.seek(currentpos);
	}

	quint16 r_fileVersion = 0;
	buffer.read(r_fileVersion);

	if ((r_fileVersion >> 8) > (lastSupportedSpecificationVersion >> 8)) {
		emit sg_LoadingFailed("File was created in a newer version of program and cannot be loaded");
		return;
	}

	if (((r_fileVersion >> 8) == (lastSupportedSpecificationVersion >> 8)) &&
		((r_fileVersion & 0x00ff) > (lastSupportedSpecificationVersion & 0x00ff))) {
		bool abort = false;

		QSemaphore s;
		emit sg_ConfirmationRequest(&s, "File was created in newer version of program."
									"Some data will not be loaded. Continue?", &abort);
		s.acquire();

		if (abort) {
			emit sg_LoadingAborted();
			return;
		}
	}

	doc->fileVersion = r_fileVersion;

	switch (r_fileVersion) {
		case 0x0001:
			load_v1(buffer);
			break;
		default:
			WARNING("Wrong case branch");
			emit sg_LoadingFailed("Unknown file version");
	}


}

void Serializer::load_v1(BOIBuffer& buffer) {
	qint64 readResult = 0;

	quint8 r_compressionLevel = 0;
	buffer.read(r_compressionLevel);

	quint8 r_cipherID = 0;
	buffer.read(r_cipherID);

	if (r_cipherID != 0 && (!Cipherer().GetAvaliableCipherIDs().contains(r_cipherID))) {
		emit sg_LoadingFailed("Cipher is not supported");
		return;
	}

	quint8 r_hashID = 0;
	quint8 r_secureHashID = 0;

	QByteArray r_cipherKey; // correct password
	if (r_cipherID > 0) {
		Cipherer c;

		buffer.read(r_hashID);
		if (!c.IsHashSupported(r_hashID)) {
			emit sg_LoadingFailed("Hash algorithm is not supported");
			return;
		}

		buffer.read(r_secureHashID);
		if (!c.IsSecureHashSupported(r_secureHashID)) {
			emit sg_LoadingFailed("Hash algorithm is not supported");
			return;
		}

		quint32 passwordHashSize = 0;
		buffer.read(passwordHashSize);

		QByteArray passwordHash(passwordHashSize, 0x0);
		buffer.read(passwordHash.data(), passwordHashSize);

		bool wrongPassword = false;

		while (true) {
			QString password;
			QSemaphore s;

			emit sg_PasswordRequired(&s, &password, wrongPassword);
			s.acquire(); // wait user action

			if (password.isEmpty()) {
				emit sg_LoadingAborted();
				return;
			} else {
				QByteArray testPasswordHash = c.GetSecureHash(password.toAscii(), r_secureHashID);
				if (passwordHash == testPasswordHash) {
					r_cipherKey = password.toAscii();
					break;
				} else {
					wrongPassword = true;
				}
			}
		}
	}

	quint32 dataBlockSize = 0;
	buffer.read(dataBlockSize);

	QByteArray dataArray(dataBlockSize, 0x0);
	buffer.read(dataArray.data(), dataBlockSize);
	buffer.close();

	if (r_cipherID != 0) {
		Cipherer c;
		QByteArray decryptionKey = c.GetHash(r_cipherKey, r_hashID);
		dataArray = c.Decrypt(dataArray, decryptionKey, r_cipherID);
		if (dataArray.isNull()) {
			emit sg_LoadingFailed("Encryption error");
			return;
		}
	}
	if (r_compressionLevel != 0) {
		Compressor c;
		dataArray = c.Decompress(dataArray);
	}


	doc->compressionLevel = r_compressionLevel;
	doc->cipherID = r_cipherID;
	doc->password = r_cipherKey;
	doc->fileName = filename;

	BOIBuffer dataBuffer(&dataArray);
	dataBuffer.open(QIODevice::ReadOnly);

	// Read document properties block
	{
		quint32 documentBlockSize = 0;
		dataBuffer.read(documentBlockSize);

		const qint64 blockStart = dataBuffer.pos();

		quint32 docCreationDate = 0;
		dataBuffer.read(docCreationDate);
		doc->creationDate = QDateTime::fromTime_t(docCreationDate);

		quint32 docModificationDate = 0;
		dataBuffer.read(docModificationDate);
		doc->modificationDate = QDateTime::fromTime_t(docModificationDate);

		quint32 defFolderIconSize = 0;
		dataBuffer.read(defFolderIconSize);
		QByteArray defFolderIcon(defFolderIconSize, 0x0);
		dataBuffer.read(defFolderIcon.data(), defFolderIconSize);
		doc->DefaultFolderIcon = defFolderIcon;

		quint32 defNoteIconSize = 0;
		dataBuffer.read(defNoteIconSize);
		QByteArray defNoteIcon(defNoteIconSize, 0x0);
		dataBuffer.read(defNoteIcon.data(), defNoteIconSize);
		doc->DefaultNoteIcon = defNoteIcon;

		const qint64 actualBlockSize = dataBuffer.pos() - blockStart;
		if (documentBlockSize > actualBlockSize) {
			dataBuffer.seek(dataBuffer.pos() + documentBlockSize - actualBlockSize);
		}
	}

	// Read user icons
	{
		quint32 userIconsBlockSize = 0;
		readResult = dataBuffer.read(userIconsBlockSize);
		qint64 blockEnd = dataBuffer.pos() + userIconsBlockSize;

		while(dataBuffer.pos() < blockEnd) {
			quint32 nameArraySize = 0;
			readResult = dataBuffer.read(nameArraySize);
			QByteArray nameArray(nameArraySize, 0x0);
			readResult = dataBuffer.read(nameArray.data(), nameArraySize);

			QFileInfo iconInfo(nameArray);
			quint32 imageDataSize = 0;
			readResult = dataBuffer.read(imageDataSize);
			QByteArray pixmapArray(imageDataSize, 0x0);
			readResult = dataBuffer.read(pixmapArray.data(), imageDataSize);

			CachedImageFile* image = new CachedImageFile(pixmapArray, nameArray, iconInfo.suffix());

			doc->AddCustomIconToStorage(image);
			sendProgressSignal(&dataBuffer);
		}
	}

	QHash<quint32, Tag*> tagsIDs;
	// Reading tags
	{
		quint32 blockSize = 0;
		readResult = dataBuffer.read(blockSize);
		const qint64 blockEndByte = dataBuffer.pos() + blockSize;

		while (dataBuffer.pos() < blockEndByte) {
			quint32 tagID = 0;
			readResult = dataBuffer.read(tagID);
			Tag* tag = Tag::Deserialize(doc->fileVersion, dataBuffer);

			tagsIDs.insert(tagID, tag);
			sendProgressSignal(&dataBuffer);
		}
	}

	QHash<quint32, AbstractFolderItem*> folderItems;
	// Read notes
	{
		quint32 blockSize = 0;
		readResult = dataBuffer.read(blockSize);
		const qint64 blockEndByte = dataBuffer.pos() + blockSize;

		while (dataBuffer.pos() < blockEndByte) {
			quint32 folderItemID = 0;
			readResult = dataBuffer.read(folderItemID);
			Note* note = Note::Deserialize(doc->fileVersion, dataBuffer);

			folderItems.insert(folderItemID, note);
			sendProgressSignal(&dataBuffer);
		}
	}

	// Read folders
	{
		quint32 blockSize = 0;
		readResult = dataBuffer.read(blockSize);

		const qint64 blockLastByte = dataBuffer.pos() + blockSize;

		quint32 folderID = 0;

		folderItems.insert(0, doc->rootFolder);
		folderItems.insert(1, doc->tempFolder);
		folderItems.insert(2, doc->trashFolder);

		// Read user folders
		while(dataBuffer.pos() < blockLastByte) {
			readResult = dataBuffer.read(folderID);
			Folder* folder = Folder::Deserialize(doc->fileVersion, dataBuffer);
			folderItems.insert(folderID, folder);

			sendProgressSignal(&dataBuffer);
		}
	}

	// Read hierarchy
	{
		quint32 blockSize = 0;
		readResult = dataBuffer.read(blockSize);

		qint64 blockLastByte = dataBuffer.pos() + blockSize;

		while (dataBuffer.pos() < blockLastByte) {
			quint32 folderID = 0;
			readResult = dataBuffer.read(folderID);
			if (!folderItems.contains(folderID)) {
				WARNING("Could not find item by ID");
				emit sg_LoadingFailed("File corrupted");
				return;
			}
			Folder* parentFolder = dynamic_cast<Folder*>(folderItems.value(folderID));
			if (!parentFolder) {
				WARNING("Casting error");
				parentFolder = doc->rootFolder;
			}

			quint32 childrenCount = 0;
			readResult = dataBuffer.read(childrenCount);

			while (childrenCount > 0) {
				childrenCount--;
				quint32 childID = 0;
				readResult = dataBuffer.read(childID);
				if (!folderItems.contains(childID)) {
					WARNING("Could not find item by ID");
					continue;
				}
				AbstractFolderItem* childItem = folderItems.value(childID);
				parentFolder->Items.Add(childItem);

				sendProgressSignal(&dataBuffer);
			}
		}
	}

	// Read tags ownership data
	{
		quint32 blockSize = 0;
		readResult = dataBuffer.read(blockSize);

		qint64 blockLastByte = dataBuffer.pos() + blockSize;

		while (dataBuffer.pos() < blockLastByte) {
			quint32 tagID = 0;
			readResult = dataBuffer.read(tagID);
			if (!tagsIDs.contains(tagID)) {
				WARNING("Could not find item by ID");
				emit sg_LoadingFailed("File corrupted");
				return;
			}
			Tag* tag = tagsIDs.value(tagID);

			quint32 ownersCount = 0;
			readResult = dataBuffer.read(ownersCount);

			while (ownersCount > 0) {
				ownersCount--;
				quint32 ownerID = 0;
				readResult = dataBuffer.read(ownerID);
				if (!folderItems.contains(ownerID)) {
					WARNING("Could not find note by ID");
					continue;
				}
				Note* note = dynamic_cast<Note*>(folderItems.value(ownerID));
				if (!note) {
					WARNING("Casting error");
					continue;
				}
				note->IsTagsListInitializationInProgress = true;
				note->Tags.Add(tag);
				note->IsTagsListInitializationInProgress = false;

				sendProgressSignal(&dataBuffer);
			}
		}
	}

	dataBuffer.close();

	doc->hasUnsavedData = false;
	emit sg_LoadingFinished();
}

void Serializer::save() {
	emit sg_SavingStarted();

	switch (version) {
		case 0x0001:
			save_v1();
			break;
		default:
			WARNING("Wrong case branch");
			emit sg_SavingFailed("Unknown file version");
	}


}

void Serializer::save_v1() {
	QByteArray fileDataArray;

	BOIBuffer fileDataBuffer(&fileDataArray);
	fileDataBuffer.open(QIODevice::WriteOnly);

	qint64 writeResult = 0;

	const char fileSignature[9] = {0x89, 0x51, 0x4E, 0x4D,
							   0x53, 0x0D, 0x0A, 0x1A, 0x0A};

	writeResult = fileDataBuffer.write(fileSignature, 9);

	writeResult = fileDataBuffer.write(doc->fileVersion);
	writeResult = fileDataBuffer.write(doc->compressionLevel);
	writeResult = fileDataBuffer.write(doc->cipherID);


	QByteArray encryptionKey;
	if (doc->cipherID > 0) {
		Cipherer c;

		quint8 r_hashID = c.DefaultHashID;
		fileDataBuffer.write(r_hashID);

		quint8 r_secureHashID = c.DefaultSecureHashID;
		fileDataBuffer.write(r_secureHashID);


		encryptionKey = c.GetHash(doc->password, r_hashID);

		const QByteArray passwordHash = c.GetSecureHash(doc->password, r_secureHashID);

		const quint32 passwordHashSize = passwordHash.size();
		writeResult = fileDataBuffer.write(passwordHashSize);
		writeResult = fileDataBuffer.write(passwordHash.constData(), passwordHashSize);
	}

	quint32 dataBlockSize = 0;
	const qint64 dataBlockSizePosition = fileDataBuffer.pos();
	writeResult = fileDataBuffer.write(dataBlockSize);

	QByteArray dataArray;
	BOIBuffer dataBuffer(&dataArray);
	dataBuffer.open(QIODevice::WriteOnly);

	// Write document data block
	{
		quint32 dtBlockSize = 0;
		const qint64 dtBlockSizePosition = dataBuffer.pos();
		dataBuffer.write(dtBlockSize);
		const quint32 docCreationDate = doc->creationDate.toTime_t();
		const quint32 docModificationDate = doc->modificationDate.toTime_t();
		dataBuffer.write(docCreationDate);
		dataBuffer.write(docModificationDate);

		const QByteArray defFolderIcon = doc->DefaultFolderIcon.toAscii();
		const quint32 defFolderIconLength = defFolderIcon.length();
		dataBuffer.write(defFolderIconLength);
		dataBuffer.write(defFolderIcon.constData(), defFolderIconLength);

		const QByteArray defNoteIcon = doc->DefaultNoteIcon.toAscii();
		const quint32 defNoteIconLength = defNoteIcon.length();
		dataBuffer.write(defNoteIconLength);
		dataBuffer.write(defNoteIcon.constData(), defNoteIconLength);

		dtBlockSize =	sizeof(docCreationDate) +
						sizeof(docModificationDate) +
						sizeof (defFolderIconLength) +
						defFolderIconLength +
						sizeof (defNoteIconLength) +
						defNoteIconLength;

		const qint64 currentPos = dataBuffer.pos();
		dataBuffer.seek(dtBlockSizePosition);
		dataBuffer.write(dtBlockSize);
		dataBuffer.seek(currentPos);
	}

	// Write user icons
	{
		quint32 iconsBlockSize = 0;
		const qint64 iconsBlockSizePosition = dataBuffer.pos();
		dataBuffer.write(iconsBlockSize);

		qint64 blockStart = dataBuffer.pos();

		foreach(QString name, doc->customIcons.keys()) {
			CachedImageFile* image = doc->customIcons.value(name);

			const QByteArray nameArray = image->GetFileName().toUtf8();
			const quint32 nameArraySize = nameArray.size();
			dataBuffer.write(nameArraySize);
			dataBuffer.write(nameArray.constData(), nameArraySize);

			const quint32 imageDataSize = image->Size();
			dataBuffer.write(imageDataSize);
			dataBuffer.write(image->GetData(), imageDataSize);
		}

		const qint64 lastPos = dataBuffer.pos();
		iconsBlockSize = lastPos - blockStart;
		dataBuffer.seek(iconsBlockSizePosition);
		dataBuffer.write(iconsBlockSize);
		dataBuffer.seek(lastPos);
	}

	// Write tags
	QHash<const Tag*, quint32> tagsIDs;
	{
		quint32 blockSize = 0;
		const qint64 blockSizePosition = dataBuffer.pos();
		dataBuffer.write(blockSize);

		const qint64 blockStartPosition = dataBuffer.pos();
		quint32 tagID = 1;
		for (int i = 0; i < doc->allTags.size(); ++i) {
			const Tag* tag = doc->allTags.value(i);
			dataBuffer.write(tagID);
			tagsIDs.insert(tag, tagID);
			tagID++;
			tag->Serialize(doc->fileVersion, dataBuffer);
		}
		const qint64 blockEndPosition = dataBuffer.pos();
		blockSize = blockEndPosition - blockStartPosition;
		dataBuffer.seek(blockSizePosition);
		dataBuffer.write(blockSize);
		dataBuffer.seek(blockEndPosition);
	}

	// Write notes
	quint32 folderOrNoteID = 10; // Reserve 0-9 for system folders and for future use
	QHash<const AbstractFolderItem*, quint32> folderItemsIDs;
	{
		quint32 blockSize = 0;
		const qint64 blockSizePosition = dataBuffer.pos();
		dataBuffer.write(blockSize);
		const qint64 blockStartPosition = dataBuffer.pos();
		for (int i = 0; i < doc->allNotes.size(); ++i) {
			const Note* note = doc->allNotes.value(i);
			dataBuffer.write(folderOrNoteID);
			folderItemsIDs.insert(note, folderOrNoteID);
			folderOrNoteID++;
			note->Serialize(doc->fileVersion, dataBuffer);
		}
		const qint64 blockEndPosition = dataBuffer.pos();
		blockSize = blockEndPosition - blockStartPosition;
		dataBuffer.seek(blockSizePosition);
		dataBuffer.write(blockSize);
		dataBuffer.seek(blockEndPosition);
	}

	// Write folders data
	{
		quint32 blockSize = 0;
		const qint64 blockSizePosition = dataBuffer.pos();
		dataBuffer.write((char*)&blockSize, sizeof(blockSize));
		const qint64 blockStartPosition = dataBuffer.pos();

		// Reserve 0-2 for system folders
		folderItemsIDs.insert(doc->rootFolder, 0);
		folderItemsIDs.insert(doc->tempFolder, 1);
		folderItemsIDs.insert(doc->trashFolder, 2);

		// Write user folders
		for (int i = 0; i < doc->allFolders.size(); ++i) {
			const Folder* f = doc->allFolders.value(i);
			dataBuffer.write(folderOrNoteID);
			folderItemsIDs.insert(f, folderOrNoteID);
			folderOrNoteID++;
			f->Serialize(doc->fileVersion, dataBuffer);
		}

		const qint64 blockEndPosition = dataBuffer.pos();
		blockSize = blockEndPosition - blockStartPosition;
		dataBuffer.seek(blockSizePosition);
		dataBuffer.write(blockSize);
		dataBuffer.seek(blockEndPosition);
	}

	// Write hierarchy
	{
		quint32 blockSize = 0;
		const qint64 blockSizePosition = dataBuffer.pos();
		dataBuffer.write(blockSize);
		const qint64 blockStartPosition = dataBuffer.pos();

		QStack<const Folder*> foldersStack;
		foldersStack.push(doc->rootFolder);
		foldersStack.push(doc->tempFolder);
		foldersStack.push(doc->trashFolder);

		while (!foldersStack.isEmpty()) {
			const Folder* folder = foldersStack.pop();
			const quint32 folderID = folderItemsIDs.value(folder);
			const quint32 childrenCount = (quint32)folder->Items.Count();
			dataBuffer.write(folderID);
			dataBuffer.write(childrenCount);
			for (int i = 0; i < folder->Items.Count(); ++i) {
				const AbstractFolderItem* child = folder->Items.ItemAt(i);
				const quint32 childID = folderItemsIDs.value(child);
				dataBuffer.write(childID);
				if (child->GetItemType() == AbstractFolderItem::Type_Folder) {
					const Folder* f = dynamic_cast<const Folder*>(child);
					foldersStack.push(f);
				}
			}
		}

		const qint64 blockEndPosition = dataBuffer.pos();
		blockSize = blockEndPosition - blockStartPosition;
		dataBuffer.seek(blockSizePosition);
		dataBuffer.write(blockSize);
		dataBuffer.seek(blockEndPosition);
	}

	// Write tags ownership data
	{
		quint32 blockSize = 0;
		const qint64 blockSizePosition = dataBuffer.pos();
		dataBuffer.write(blockSize);
		const qint64 blockStartPosition = dataBuffer.pos();
		{
			foreach (const Tag* tag, doc->allTags) {
				const quint32 tagID = tagsIDs.value(tag);
				const quint32 ownersCount = tag->Owners.Count();
				dataBuffer.write(tagID);
				dataBuffer.write(ownersCount);
				for (int i = 0; i < (int)ownersCount; ++i) {
					const Note* note = tag->Owners.ItemAt(i);
					const quint32 ownerID = folderItemsIDs.value(note);
					dataBuffer.write(ownerID);
				}
			}
		}
		const qint64 blockEndPosition = dataBuffer.pos();
		blockSize = blockEndPosition - blockStartPosition;
		dataBuffer.seek(blockSizePosition);
		dataBuffer.write(blockSize);
		dataBuffer.seek(blockEndPosition);
	}


	dataBuffer.close();

	if (doc->compressionLevel > 0) {
		Compressor c;
		dataArray = c.Compress(dataArray, doc->compressionLevel);
	}
	if (doc->cipherID > 0) {
		if (!Cipherer().GetAvaliableCipherIDs().contains(doc->cipherID)) {
			emit sg_SavingFailed("Cipher is not supported");
			return;
		}
		Cipherer c;
		dataArray = c.Encrypt(dataArray, encryptionKey, doc->cipherID);
		if (dataArray.isNull()) {
			emit sg_SavingFailed("Encryption error");
			return;
		}
	}

	fileDataBuffer.write(dataArray);
	quint64 lastPos = fileDataBuffer.pos();

	// Write data block size
	dataBlockSize = dataArray.size();
	fileDataBuffer.seek(dataBlockSizePosition);
	fileDataBuffer.write(dataBlockSize);

	// Write file crc
	quint32 crc = crc32buf(fileDataArray.constData(), fileDataArray.size());
	fileDataBuffer.seek(lastPos);
	fileDataBuffer.write(crc);

	fileDataBuffer.close();


	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly)) {
		emit sg_SavingFailed("Cannot open file for writing");
		return;
	}

	writeResult = file.write(fileDataArray.constData(), fileDataArray.size());
	file.close();
	if (writeResult != fileDataArray.size()) {
		if (file.exists()) {
			file.remove();
		}
	}

	doc->hasUnsavedData = false;
	if (doc->fileName != filename) {doc->fileName = filename;}
	if (doc->fileVersion != version) {doc->fileVersion = version;}
	emit sg_SavingFinished();
}

void Serializer::sendProgressSignal(BOIBuffer* buffer) {
	if (!buffer) {
		WARNING("Null pointer recieved");
		return;
	}
	int progress = buffer->size() == 0 ? 0 : (buffer->pos() * 100) / buffer->size();
	//qDebug() << "Progress:" << progress;

	emit sg_LoadingProgress(progress);
}
