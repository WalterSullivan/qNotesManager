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
#include "textdocument.h"

#include <QFile>
#include <QFileInfo>
#include <QApplication>
#include <QStack>

using namespace qNotesManager;

Serializer::Serializer() : QObject(0) {
	doc = 0;
	operation = Unknown;
	filename = QString();
}

void Serializer::Load(Document* d, const QString& fileNameToLoad) {
	doc = d;
	filename = fileNameToLoad;
	operation = Loading;
}

void Serializer::Save(Document* d, const QString& fileNameToSave, quint16 version) {
	doc = d;
	filename = fileNameToSave;
	operation = Saving;
	saveVersion = version;
}

void Serializer::sl_start() {
	if (!doc || operation == Unknown || filename.isEmpty()) {
		WARNING("Wrong argument");
		emit sg_finished();
		return;
	}

	switch (operation) {
	case Loading:
		loadDocument();
		break;
	case Saving:
		saveDocument();
		break;
	case Unknown:
	default:
		WARNING("Wrong case branch");
		break;

	}

	emit sg_finished();
}

void Serializer::loadDocument() {
	doc->inInitMode = true;

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

		r_fileVersion = lastSupportedSpecificationVersion;
	}

	doc->fileVersion = r_fileVersion;

	switch (r_fileVersion) {
		case 0x0001:
			loadDocument_v1(buffer);
			break;
		case 0x0002:
			loadDocument_v2(buffer);
			break;
		default:
			WARNING("Wrong case branch");
			emit sg_LoadingFailed("Unknown file version");
	}

	doc->inInitMode = false;
}

void Serializer::saveDocument() {
	emit sg_SavingStarted();

	if (saveVersion > lastSupportedSpecificationVersion) {
		bool abort = false;

		QSemaphore s;
		emit sg_ConfirmationRequest(&s, "Specified save version is not supported."
									"Use last supported version?", &abort);
		s.acquire();

		if (abort) {
			emit sg_SavingAborted();
			return;
		}

		saveVersion = lastSupportedSpecificationVersion;
	}

	switch (saveVersion) {
		case 0x0001:
			saveDocument_v1();
			break;
		case 0x0002:
			saveDocument_v2();
			break;
		default:
			WARNING("Wrong case branch");
			emit sg_SavingFailed("Unknown file version");
	}
}

void Serializer::sendProgressSignal(BOIBuffer* buffer) {
	if (!buffer) {
		WARNING("Null pointer recieved");
		return;
	}
	int progress = buffer->size() == 0 ? 0 : (buffer->pos() * 100) / buffer->size();

	emit sg_LoadingProgress(progress);
}

// Save file ver. 1
void Serializer::loadDocument_v1(BOIBuffer& buffer) {
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
			Tag* tag = loadTag_v1(dataBuffer);

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
			Note* note = loadNote_v1(dataBuffer);

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
			Folder* folder = loadFolder_v1(dataBuffer);
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

	emit sg_LoadingFinished();
}

void Serializer::saveDocument_v1() {
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
			saveTag_v1(tag, dataBuffer);
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
			saveNote_v1(note, dataBuffer);
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
			saveFolder_v1(f, dataBuffer);
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
	if (doc->fileVersion != saveVersion) {doc->fileVersion = saveVersion;}
	emit sg_SavingFinished();
}

Note* Serializer::loadNote_v1(BOIBuffer& buffer) {
	qint64 bytesRead = 0;

	quint32 r_itemSize = 0;
	bytesRead = buffer.read(r_itemSize);

	const qint64 dataStartPos = buffer.pos();

	quint32 r_captionSize = 0;
	bytesRead = buffer.read(r_captionSize);
	QByteArray r_captionArray(r_captionSize, 0x0);
	bytesRead = buffer.read(r_captionArray.data(), r_captionSize);
	quint32 r_textSize = 0;
	bytesRead = buffer.read(r_textSize);
	QByteArray r_textArray(r_textSize, 0x0);
	bytesRead = buffer.read(r_textArray.data(), r_textSize);
	quint32 r_creationDate = 0;
	bytesRead = buffer.read(r_creationDate);
	quint32 r_modificationDate = 0;
	bytesRead = buffer.read(r_modificationDate);
	quint32 r_textDate = 0;
	bytesRead = buffer.read(r_textDate);
	quint32 r_authorSize = 0;
	bytesRead = buffer.read(r_authorSize);
	QByteArray r_authorArray(r_authorSize, 0x0);
	bytesRead = buffer.read(r_authorArray.data(), r_authorSize);
	quint32 r_sourceSize = 0;
	bytesRead = buffer.read(r_sourceSize);
	QByteArray r_sourceArray(r_sourceSize, 0x0);
	bytesRead = buffer.read(r_sourceArray.data(), r_sourceSize);
	quint32 r_commentSize = 0;
	bytesRead = buffer.read(r_commentSize);
	QByteArray r_commentArray(r_commentSize, 0x0);
	bytesRead = buffer.read(r_commentArray.data(),			r_commentSize);
	quint32 r_iconIDSize = 0;
	bytesRead = buffer.read(r_iconIDSize);
	QByteArray r_iconID(r_iconIDSize, 0x0);
	bytesRead = buffer.read(r_iconID.data(),			r_iconIDSize);
	quint32 r_backColor = 0;
	bytesRead = buffer.read(r_backColor);
	quint32 r_foreColor = 0;
	bytesRead = buffer.read(r_foreColor);
	quint8 r_locked = 0;
	bytesRead = buffer.read(r_locked);
	quint32 r_imagesListSize = 0;
	bytesRead = buffer.read(r_imagesListSize);

	QList<CachedImageFile*> images;

	if (r_imagesListSize > 0) {
		quint32 imagesSize = 0;

		while(imagesSize < r_imagesListSize) {
			quint32 r_imageNameSize = 0;
			bytesRead = buffer.read(r_imageNameSize);
			QByteArray r_imageName(r_imageNameSize, 0x0);
			bytesRead = buffer.read(r_imageName.data(), r_imageNameSize);

			quint32 imageFormatSize = 0;
			bytesRead = buffer.read(imageFormatSize);
			QByteArray imageFormat(imageFormatSize, 0x0);
			bytesRead = buffer.read(imageFormat.data(), imageFormatSize);

			quint32 r_imageArraySize = 0;
			bytesRead = buffer.read(r_imageArraySize);
			QByteArray r_imageArray(r_imageArraySize, 0x0);
			bytesRead = buffer.read(r_imageArray.data(), r_imageArraySize);

			CachedImageFile* image = new CachedImageFile(r_imageArray, r_imageName, imageFormat);
			images.push_back(image);

			imagesSize +=	sizeof(r_imageNameSize) +
							r_imageNameSize +
							sizeof(imageFormatSize) +
							imageFormatSize +
							sizeof(r_imageArraySize) +
							r_imageArraySize;
		}
	}

	const quint32 bytesToSkip = r_itemSize - (buffer.pos() - dataStartPos);

	if (bytesToSkip != 0) {
		// If block has more data in case of newer file version.
		buffer.seek(buffer.pos() + bytesToSkip);
	}

	Note* note = new Note("");
	note->name = r_captionArray;
	note->creationDate = QDateTime::fromTime_t(r_creationDate);
	note->modificationDate = QDateTime::fromTime_t(r_modificationDate);
	note->textDate = r_textDate == 0 ? QDateTime() : QDateTime::fromTime_t(r_textDate);
	note->author = r_authorArray;
	note->source = r_sourceArray;
	note->comment = r_commentArray;
	note->iconID = r_iconID;
	note->nameBackColor.setRgba(r_backColor);
	note->nameForeColor.setRgba(r_foreColor);
	note->locked = (bool)r_locked;
	note->cachedHtml = r_textArray;
	note->textDocumentInitialized = false;
	foreach(CachedImageFile* image, images) {
		note->document->AddResourceImage(image);
	}

	return note;
}

void Serializer::saveNote_v1(const Note* note, BOIBuffer& buffer) {
	const QByteArray w_captionArray = note->name.toUtf8();
	const quint32 w_captionSize = w_captionArray.size();
	const QByteArray w_textArray = note->cachedHtml.isNull() ?
								   note->document->toHtml().toUtf8() : note->cachedHtml.toUtf8();
	const quint32 w_textSize = w_textArray.size();
	const quint32 w_creationDate = note->creationDate.toTime_t();
	const quint32 w_modificationDate = note->modificationDate.toTime_t();
	const quint32 w_textDate = note->textDate.isValid() ? note->textDate.toTime_t() : 0;
	const QByteArray w_iconID = note->iconID.toAscii();
	const quint32 w_iconIDSize = w_iconID.size();
	const QByteArray w_authorArray = note->author.toUtf8();
	const quint32 w_authorSize = w_authorArray.size();
	const QByteArray w_sourceArray = note->source.toUtf8();
	const quint32 w_sourceSize = w_sourceArray.size();
	const QByteArray w_commentArray = note->comment.toUtf8();
	const quint32 w_commentSize = w_commentArray.size();
	const quint32 w_backColor = note->nameBackColor.rgba();
	const quint32 w_foreColor = note->nameForeColor.rgba();
	const quint8 w_locked = (quint8)note->locked;

	// Write images to temporary buffer
	QStringList imagesNamesList;
	if (note->textDocumentInitialized) {
		imagesNamesList = note->document->GetImagesList();
	} else {
		imagesNamesList = note->document->GetResourceImagesList();
	}

	QByteArray imagesArray;
	BOIBuffer imagesArrayBuffer(&imagesArray);
	imagesArrayBuffer.open(QIODevice::WriteOnly);

	foreach (QString imageName, imagesNamesList) {
		CachedImageFile* image = note->document->GetResourceImage(imageName);
		if (!image) {
			WARNING("Image not found");
			continue;
		}

		QByteArray imageNameArray = image->GetFileName().toUtf8();
		const quint32 imageNameSize = imageNameArray.size();

		const QByteArray formatArray = image->GetFormat().toAscii();
		const quint32 formatArraySize = formatArray.size();

		// writing data
		imagesArrayBuffer.write(imageNameSize);
		imagesArrayBuffer.write(imageNameArray.constData(), imageNameSize);

		imagesArrayBuffer.write(formatArraySize);
		imagesArrayBuffer.write(formatArray.constData(), formatArraySize);

		const quint32 imageArraySize = image->Size();
		imagesArrayBuffer.write(imageArraySize);
		imagesArrayBuffer.write(image->GetData(), imageArraySize);
	}
	imagesArrayBuffer.close();


	const quint32 w_imagesArraySize = imagesArray.size();
	const quint32 w_itemSize =	sizeof(w_captionSize) +
								w_captionSize +
								sizeof(w_textSize) +
								w_textSize +
								sizeof(w_creationDate) +
								sizeof(w_modificationDate) +
								sizeof(w_textDate) +
								w_iconIDSize +
								sizeof(w_iconIDSize) +
								sizeof(w_authorSize) +
								w_authorSize +
								sizeof(w_sourceSize) +
								w_sourceSize +
								sizeof(w_commentSize) +
								w_commentSize +
								sizeof(w_backColor) +
								sizeof(w_foreColor) +
								sizeof(w_locked) +
								sizeof(w_imagesArraySize) +
								w_imagesArraySize;
	qint64 result = 0;
	result = buffer.write(w_itemSize);
	result = buffer.write(w_captionSize);
	result = buffer.write(w_captionArray.constData(),	w_captionSize);
	result = buffer.write(w_textSize);
	result = buffer.write(w_textArray.constData(),		w_textSize);
	result = buffer.write(w_creationDate);
	result = buffer.write(w_modificationDate);
	result = buffer.write(w_textDate);
	result = buffer.write(w_authorSize);
	result = buffer.write(w_authorArray.constData(),	w_authorSize);
	result = buffer.write(w_sourceSize);
	result = buffer.write(w_sourceArray.constData(),	w_sourceSize);
	result = buffer.write(w_commentSize);
	result = buffer.write(w_commentArray.constData(),	w_commentSize);
	result = buffer.write(w_iconIDSize);
	result = buffer.write(w_iconID.constData(), w_iconIDSize);
	result = buffer.write(w_backColor);
	result = buffer.write(w_foreColor);
	result = buffer.write(w_locked);
	result = buffer.write(w_imagesArraySize);
	result = buffer.write(imagesArray);
}

Folder* Serializer::loadFolder_v1(BOIBuffer& buffer) {
	qint64 bytesRead = 0;

	quint32 r_itemSize = 0;
	bytesRead = buffer.read(r_itemSize);

	const qint64 bufferStartPos = buffer.pos();

	quint32 r_captionSize = 0;
	bytesRead = buffer.read(r_captionSize);
	QByteArray r_caption(r_captionSize, 0x0);
	bytesRead = buffer.read(r_caption.data(), r_captionSize);
	quint32 r_creationDate = 0;
	bytesRead = buffer.read(r_creationDate);
	quint32 r_modificationDate = 0;
	bytesRead = buffer.read(r_modificationDate);
	quint32 r_iconIDSize = 0;
	bytesRead = buffer.read(r_iconIDSize);
	QByteArray r_iconID(r_iconIDSize, 0x0);
	bytesRead = buffer.read(r_iconID.data(), r_iconIDSize);
	quint32 r_backColor = 0;
	bytesRead = buffer.read(r_backColor);
	quint32 r_foreColor = 0;
	bytesRead = buffer.read(r_foreColor);
	quint8 r_locked = 0;
	bytesRead = buffer.read(r_locked);

	const quint32 bytesToSkip = r_itemSize - (buffer.pos() - bufferStartPos);

	if (bytesToSkip != 0) {
		// If chunck has more data in case of newer file version.
		buffer.seek(buffer.pos() + bytesToSkip);
	}

	Folder* folder = new Folder("");
	folder->name = r_caption;
	folder->nameForeColor.setRgba(r_foreColor);
	folder->nameBackColor.setRgba(r_backColor);
	folder->locked = (bool)r_locked;
	folder->iconID = r_iconID;
	folder->creationDate = QDateTime::fromTime_t(r_creationDate);
	folder->modificationDate = QDateTime::fromTime_t(r_modificationDate);

	return folder;
}

void Serializer::saveFolder_v1(const Folder* folder, BOIBuffer& buffer) {
	const QByteArray s_caption = folder->name.toUtf8();
	const quint32 s_captionSize = s_caption.size();
	const quint32 s_creationDate = folder->creationDate.toTime_t();
	const quint32 s_modificationDate = folder->modificationDate.toTime_t();
	const QByteArray s_iconID = folder->iconID.toAscii();
	const quint32 s_iconIDSize = s_iconID.size();
	const quint32 s_backColor = folder->nameBackColor.rgba();
	const quint32 s_foreColor = folder->nameForeColor.rgba();
	const quint8 s_locked = (quint8)folder->locked;
	const quint32 s_itemSize =	s_captionSize +
								sizeof(s_captionSize) +
								sizeof(s_creationDate) +
								sizeof(s_modificationDate) +
								s_iconIDSize +
								sizeof(s_iconIDSize) +
								sizeof(s_backColor) +
								sizeof(s_foreColor) +
								sizeof(s_locked);

	buffer.write(s_itemSize);
	buffer.write(s_captionSize);
	buffer.write(s_caption.constData(), s_captionSize);
	buffer.write(s_creationDate);
	buffer.write(s_modificationDate);
	buffer.write(s_iconIDSize);
	buffer.write(s_iconID.constData(), s_iconIDSize);
	buffer.write(s_backColor);
	buffer.write(s_foreColor);
	buffer.write(s_locked);
}

Tag* Serializer::loadTag_v1(BOIBuffer& buffer) {
	qint64 result = 0;

	quint32 r_itemSize = 0;
	result = buffer.read(r_itemSize);

	const qint64 streamStartPos = buffer.pos();

	quint32 r_nameSize = 0;
	result = buffer.read(r_nameSize);

	QByteArray r_nameArray(r_nameSize, 0x0);
	result = buffer.read(r_nameArray.data(), r_nameSize);

	const quint32 bytesToSkip = r_itemSize - (buffer.pos() - streamStartPos);

	if (bytesToSkip != 0) {
		buffer.seek(buffer.pos() + bytesToSkip); // If chunck has more data in case of newer file version.
	}

	return new Tag(r_nameArray);
}

void Serializer::saveTag_v1(const Tag* tag, BOIBuffer& buffer) {
	const QByteArray w_nameArray = tag->_name.toUtf8();
	const quint32 w_nameSize = w_nameArray.size();
	const quint32 w_itemSize = w_nameSize + sizeof(w_nameSize);

	qint64 result = 0;
	result = buffer.write(w_itemSize);
	result = buffer.write(w_nameSize);
	result = buffer.write(w_nameArray.constData(), w_nameSize);
}

// Save file ver. 2
void Serializer::loadDocument_v2(BOIBuffer& buffer) {
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
			Tag* tag = loadTag_v2(dataBuffer);

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
			Note* note = loadNote_v2(dataBuffer);

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
			Folder* folder = loadFolder_v2(dataBuffer);
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

	// Load bookmarks
	{
		quint32 bookmarksCount = 0;
		readResult = dataBuffer.read(bookmarksCount);
		for (quint32 i = 0; i < bookmarksCount; i++) {
			quint32 bookmarkID = 0;
			dataBuffer.read(bookmarkID);

			if (!folderItems.contains(bookmarkID)) {
				WARNING("Could not find note by ID");
				continue;
			}
			Note* bookmark = dynamic_cast<Note*>(folderItems.value(bookmarkID));
			if (bookmark == 0) {
				WARNING("Could not find note by ID");
				continue;
			}

			doc->AddBookmark(bookmark);
		}
	}

	dataBuffer.close();

	emit sg_LoadingFinished();
}

void Serializer::saveDocument_v2() {
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
			saveTag_v2(tag, dataBuffer);
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
			saveNote_v2(note, dataBuffer);
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
			saveFolder_v2(f, dataBuffer);
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

	// Write bookmarks
	{
		quint32 bookmarksCount = doc->bookmarks.count();
		dataBuffer.write(bookmarksCount);
		foreach(Note* note, doc->bookmarks) {
			quint32 bookmarkID = folderItemsIDs[note];
			dataBuffer.write(bookmarkID);
		}
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
	if (doc->fileVersion != saveVersion) {doc->fileVersion = saveVersion;}
	emit sg_SavingFinished();
}

Note* Serializer::loadNote_v2(BOIBuffer& buffer) {
	qint64 bytesRead = 0;

	quint32 r_itemSize = 0;
	bytesRead = buffer.read(r_itemSize);

	const qint64 dataStartPos = buffer.pos();

	quint32 r_captionSize = 0;
	bytesRead = buffer.read(r_captionSize);
	QByteArray r_captionArray(r_captionSize, 0x0);
	bytesRead = buffer.read(r_captionArray.data(), r_captionSize);
	quint32 r_textSize = 0;
	bytesRead = buffer.read(r_textSize);
	QByteArray r_textArray(r_textSize, 0x0);
	bytesRead = buffer.read(r_textArray.data(), r_textSize);
	quint32 r_creationDate = 0;
	bytesRead = buffer.read(r_creationDate);
	quint32 r_modificationDate = 0;
	bytesRead = buffer.read(r_modificationDate);
	quint32 r_textDate = 0;
	bytesRead = buffer.read(r_textDate);
	quint32 r_authorSize = 0;
	bytesRead = buffer.read(r_authorSize);
	QByteArray r_authorArray(r_authorSize, 0x0);
	bytesRead = buffer.read(r_authorArray.data(), r_authorSize);
	quint32 r_sourceSize = 0;
	bytesRead = buffer.read(r_sourceSize);
	QByteArray r_sourceArray(r_sourceSize, 0x0);
	bytesRead = buffer.read(r_sourceArray.data(), r_sourceSize);
	quint32 r_commentSize = 0;
	bytesRead = buffer.read(r_commentSize);
	QByteArray r_commentArray(r_commentSize, 0x0);
	bytesRead = buffer.read(r_commentArray.data(),			r_commentSize);
	quint32 r_iconIDSize = 0;
	bytesRead = buffer.read(r_iconIDSize);
	QByteArray r_iconID(r_iconIDSize, 0x0);
	bytesRead = buffer.read(r_iconID.data(),			r_iconIDSize);
	quint32 r_backColor = 0;
	bytesRead = buffer.read(r_backColor);
	quint32 r_foreColor = 0;
	bytesRead = buffer.read(r_foreColor);
	quint8 r_locked = 0;
	bytesRead = buffer.read(r_locked);
	quint32 r_imagesListSize = 0;
	bytesRead = buffer.read(r_imagesListSize);

	QList<CachedImageFile*> images;

	if (r_imagesListSize > 0) {
		quint32 imagesSize = 0;

		while(imagesSize < r_imagesListSize) {
			quint32 r_imageNameSize = 0;
			bytesRead = buffer.read(r_imageNameSize);
			QByteArray r_imageName(r_imageNameSize, 0x0);
			bytesRead = buffer.read(r_imageName.data(), r_imageNameSize);

			quint32 imageFormatSize = 0;
			bytesRead = buffer.read(imageFormatSize);
			QByteArray imageFormat(imageFormatSize, 0x0);
			bytesRead = buffer.read(imageFormat.data(), imageFormatSize);

			quint32 r_imageArraySize = 0;
			bytesRead = buffer.read(r_imageArraySize);
			QByteArray r_imageArray(r_imageArraySize, 0x0);
			bytesRead = buffer.read(r_imageArray.data(), r_imageArraySize);

			CachedImageFile* image = new CachedImageFile(r_imageArray, r_imageName, imageFormat);
			images.push_back(image);

			imagesSize +=	sizeof(r_imageNameSize) +
							r_imageNameSize +
							sizeof(imageFormatSize) +
							imageFormatSize +
							sizeof(r_imageArraySize) +
							r_imageArraySize;
		}
	}

	// Load attached files
	QList<CachedFile*> attachedFiles;
	quint32 r_filesArraySize = 0;
	bytesRead = buffer.read(r_filesArraySize);
	if (r_filesArraySize > 0) {
		quint32 loadedDataSize = 0;

		while (loadedDataSize < r_filesArraySize) {
			quint32 r_fileNameSize = 0;
			bytesRead = buffer.read(r_fileNameSize);

			QByteArray r_fileName(r_fileNameSize, 0x0);
			bytesRead = buffer.read(r_fileName.data(), r_fileNameSize);

			quint32 r_fileArraySize = 0;
			bytesRead = buffer.read(r_fileArraySize);

			QByteArray r_fileArray(r_fileArraySize, 0x0);
			bytesRead = buffer.read(r_fileArray.data(), r_fileArraySize);

			CachedFile* file = new CachedFile(r_fileArray, r_fileName);
			attachedFiles.push_back(file);

			loadedDataSize +=	sizeof(r_fileNameSize) +
								r_fileNameSize +
								sizeof(r_fileArraySize) +
								r_fileArraySize;
		}
	}

	const quint32 bytesToSkip = r_itemSize - (buffer.pos() - dataStartPos);

	if (bytesToSkip != 0) {
		// If block has more data in case of newer file version.
		buffer.seek(buffer.pos() + bytesToSkip);
	}

	Note* note = new Note("");
	note->name = r_captionArray;
	note->creationDate = QDateTime::fromTime_t(r_creationDate);
	note->modificationDate = QDateTime::fromTime_t(r_modificationDate);
	note->textDate = r_textDate == 0 ? QDateTime() : QDateTime::fromTime_t(r_textDate);
	note->author = r_authorArray;
	note->source = r_sourceArray;
	note->comment = r_commentArray;
	note->iconID = r_iconID;
	note->nameBackColor.setRgba(r_backColor);
	note->nameForeColor.setRgba(r_foreColor);
	note->locked = (bool)r_locked;
	note->cachedHtml = r_textArray;
	note->textDocumentInitialized = false;
	foreach(CachedImageFile* image, images) {
		note->document->AddResourceImage(image);
	}
	foreach(CachedFile* file, attachedFiles) {
		note->attachedFiles.push_back(file);
	}

	return note;
}

void Serializer::saveNote_v2(const Note* note, BOIBuffer& buffer) {
	const QByteArray w_captionArray = note->name.toUtf8();
	const quint32 w_captionSize = w_captionArray.size();
	const QByteArray w_textArray = note->cachedHtml.isNull() ?
								   note->document->toHtml().toUtf8() : note->cachedHtml.toUtf8();
	const quint32 w_textSize = w_textArray.size();
	const quint32 w_creationDate = note->creationDate.toTime_t();
	const quint32 w_modificationDate = note->modificationDate.toTime_t();
	const quint32 w_textDate = note->textDate.isValid() ? note->textDate.toTime_t() : 0;
	const QByteArray w_iconID = note->iconID.toAscii();
	const quint32 w_iconIDSize = w_iconID.size();
	const QByteArray w_authorArray = note->author.toUtf8();
	const quint32 w_authorSize = w_authorArray.size();
	const QByteArray w_sourceArray = note->source.toUtf8();
	const quint32 w_sourceSize = w_sourceArray.size();
	const QByteArray w_commentArray = note->comment.toUtf8();
	const quint32 w_commentSize = w_commentArray.size();
	const quint32 w_backColor = note->nameBackColor.rgba();
	const quint32 w_foreColor = note->nameForeColor.rgba();
	const quint8 w_locked = (quint8)note->locked;

	// Write images to temporary buffer
	QStringList imagesNamesList;
	if (note->textDocumentInitialized) {
		imagesNamesList = note->document->GetImagesList();
	} else {
		imagesNamesList = note->document->GetResourceImagesList();
	}

	QByteArray imagesArray;
	BOIBuffer imagesArrayBuffer(&imagesArray);
	imagesArrayBuffer.open(QIODevice::WriteOnly);

	foreach (QString imageName, imagesNamesList) {
		CachedImageFile* image = note->document->GetResourceImage(imageName);
		if (!image) {
			WARNING("Image not found");
			continue;
		}

		QByteArray imageNameArray = image->GetFileName().toUtf8();
		const quint32 imageNameSize = imageNameArray.size();

		const QByteArray formatArray = image->GetFormat().toAscii();
		const quint32 formatArraySize = formatArray.size();

		// writing data
		imagesArrayBuffer.write(imageNameSize);
		imagesArrayBuffer.write(imageNameArray.constData(), imageNameSize);

		imagesArrayBuffer.write(formatArraySize);
		imagesArrayBuffer.write(formatArray.constData(), formatArraySize);

		const quint32 imageArraySize = image->Size();
		imagesArrayBuffer.write(imageArraySize);
		imagesArrayBuffer.write(image->GetData(), imageArraySize);
	}
	imagesArrayBuffer.close();

	QByteArray attachedFilesArray;
	BOIBuffer attachedFilesArrayBuffer(&attachedFilesArray);
	attachedFilesArrayBuffer.open(QIODevice::WriteOnly);

	foreach (CachedFile* file, note->attachedFiles) {
		QByteArray fileNameArray = file->GetFileName().toUtf8();
		const quint32 fileNameSize = fileNameArray.size();

		// writing data
		attachedFilesArrayBuffer.write(fileNameSize);
		attachedFilesArrayBuffer.write(fileNameArray.constData(), fileNameSize);

		const quint32 fileArraySize = file->Size();
		attachedFilesArrayBuffer.write(fileArraySize);
		attachedFilesArrayBuffer.write(file->GetData(), fileArraySize);
	}
	attachedFilesArrayBuffer.close();


	const quint32 w_imagesArraySize = imagesArray.size();
	const quint32 w_fileaArraySize = attachedFilesArray.size();
	const quint32 w_itemSize =	sizeof(w_captionSize) +
								w_captionSize +
								sizeof(w_textSize) +
								w_textSize +
								sizeof(w_creationDate) +
								sizeof(w_modificationDate) +
								sizeof(w_textDate) +
								w_iconIDSize +
								sizeof(w_iconIDSize) +
								sizeof(w_authorSize) +
								w_authorSize +
								sizeof(w_sourceSize) +
								w_sourceSize +
								sizeof(w_commentSize) +
								w_commentSize +
								sizeof(w_backColor) +
								sizeof(w_foreColor) +
								sizeof(w_locked) +
								sizeof(w_imagesArraySize) +
								w_imagesArraySize +
								sizeof(w_fileaArraySize) +
								w_fileaArraySize;
	qint64 result = 0;
	result = buffer.write(w_itemSize);
	result = buffer.write(w_captionSize);
	result = buffer.write(w_captionArray.constData(),	w_captionSize);
	result = buffer.write(w_textSize);
	result = buffer.write(w_textArray.constData(),		w_textSize);
	result = buffer.write(w_creationDate);
	result = buffer.write(w_modificationDate);
	result = buffer.write(w_textDate);
	result = buffer.write(w_authorSize);
	result = buffer.write(w_authorArray.constData(),	w_authorSize);
	result = buffer.write(w_sourceSize);
	result = buffer.write(w_sourceArray.constData(),	w_sourceSize);
	result = buffer.write(w_commentSize);
	result = buffer.write(w_commentArray.constData(),	w_commentSize);
	result = buffer.write(w_iconIDSize);
	result = buffer.write(w_iconID.constData(), w_iconIDSize);
	result = buffer.write(w_backColor);
	result = buffer.write(w_foreColor);
	result = buffer.write(w_locked);
	result = buffer.write(w_imagesArraySize);
	result = buffer.write(imagesArray);
	result = buffer.write(w_fileaArraySize);
	result = buffer.write(attachedFilesArray);
}

Folder* Serializer::loadFolder_v2(BOIBuffer& buffer) {
	return loadFolder_v1(buffer);
}

void Serializer::saveFolder_v2(const Folder* folder, BOIBuffer& buffer) {
	saveFolder_v1(folder, buffer);
}

Tag* Serializer::loadTag_v2(BOIBuffer& buffer) {
	return loadTag_v1(buffer);
}

void Serializer::saveTag_v2(const Tag* tag, BOIBuffer& buffer) {
	saveTag_v1(tag, buffer);
}
