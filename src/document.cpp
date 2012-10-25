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

#include "document.h"
#include "folder.h"
#include "abstractfolderitem.h"
#include "note.h"
#include "tag.h"
#include "application.h"
#ifdef DEBUG
#include "tracelogger.h"
#endif
#include "cipherer.h"
#include "compressor.h"

#include "invaliddataexception.h"
#include "ioexception.h"
#include "operationabortedexception.h"

#include "hierarchymodel.h"
#include "tagsmodel.h"
#include "datesmodel.h"

#include "boibuffer.h"
#include "crc32.h"

#include <QFile>
#include <QFileInfo>
#include <QBuffer>
#include <QDebug>
#include <QStack>
#include <QInputDialog>
#include <QMessageBox>


using namespace qNotesManager;

Document::Document() : QObject(0) {
	rootFolder = new Folder("_root_", Folder::SystemFolder);
	QObject::connect(rootFolder, SIGNAL(sg_ItemAdded(AbstractFolderItem*const, int)),
					 this, SLOT(sl_Folder_ItemAdded(AbstractFolderItem* const, int)));
	QObject::connect(rootFolder, SIGNAL(sg_ItemRemoved(AbstractFolderItem*const)),
					 this, SLOT(sl_Folder_ItemRemoved(AbstractFolderItem*const)));


	tempFolder = new Folder("Temporary", Folder::TempFolder);
	QObject::connect(tempFolder, SIGNAL(sg_ItemAdded(AbstractFolderItem*const, int)),
					 this, SLOT(sl_Folder_ItemAdded(AbstractFolderItem* const, int)));
	QObject::connect(tempFolder, SIGNAL(sg_ItemRemoved(AbstractFolderItem*const)),
					 this, SLOT(sl_Folder_ItemRemoved(AbstractFolderItem*const)));

	trashFolder = new Folder("Trash", Folder::TrashFolder);
	QObject::connect(trashFolder, SIGNAL(sg_ItemAdded(AbstractFolderItem*const, int)),
					 this, SLOT(sl_Folder_ItemAdded(AbstractFolderItem* const, int)));
	QObject::connect(trashFolder, SIGNAL(sg_ItemRemoved(AbstractFolderItem*const)),
					 this, SLOT(sl_Folder_ItemRemoved(AbstractFolderItem*const)));

	tagsListModel = new QStandardItemModel(this);

	hierarchyModel = new HierarchyModel(this);

	tagsModel = new TagsModel(this);

	creationDateModel = new DatesModel(DatesModel::CreationDate, this);

	modificationDateModel = new DatesModel(DatesModel::ModifyDate, this);

	textDateModel = new DatesModel(DatesModel::TextDate, this);


	LockFolderItems = true;

	// Fill customIconsModel with stock icons
	customIconsModel = new QStandardItemModel(this);
	QHash<QString, QPixmap> iconsList = Application::I()->GetStandardIcons();
	foreach (QString key, iconsList.keys()) {
		QStandardItem* iconItem = new QStandardItem(iconsList.value(key), QString());
		iconItem->setData(key, Qt::UserRole + 1);
		iconItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		customIconsModel->appendRow(iconItem);
	}

	fileName = QString();
	fileVersion = currentFileVersion;
	compressionLevel = 0;
	cipherID = 0;
	password = QByteArray();
	creationDate = QDateTime::currentDateTime();
	modificationDate = QDateTime::currentDateTime();
	isChanged = true;


	DefaultFolderIcon = Application::I()->DefaultFolderIcon;
	DefaultNoteIcon = Application::I()->DefaultNoteIcon;
}

Document::~Document() {
	delete rootFolder;
	delete trashFolder;
	delete tempFolder;
}

QString Document::GetFilename() const {
	return fileName;
}

QDateTime Document::GetCreationDate() const {
	return creationDate;
}

QDateTime Document::GetModificationDate() const {
	return modificationDate;
}

Folder* Document::GetTempFolder() const {
	return tempFolder;
}

Folder* Document::GetTrashFolder() const {
	return trashFolder;
}

Folder* Document::GetRoot() const {
	return rootFolder;
}

void Document::onChange() {
	modificationDate = QDateTime::currentDateTime();

	if (!isChanged) {
		isChanged = true;
		emit sg_Changed();
	}
}

bool Document::IsChanged() const {
	return isChanged;
}

/*static*/
Document* Document::Open(QString fileName) {
#ifdef DEBUG
		Q_ASSERT(!fileName.isEmpty());
#else
	if (fileName.isEmpty()) {
		throw InvalidDataException("Specify valid filename to load", "Filename argument is empty", WHERE);
	}
#endif


	QFile file(fileName);
#ifdef DEBUG
	Q_ASSERT(file.exists());
#else
	if (!file.exists()) {
		throw InvalidDataException("Specify valid filename to load", "Filename argument is empty", WHERE);
	}
#endif


	if (file.size() < 13) { // must me guaranteed program can read 9 bytes of signature and fileSize data
		throw WrongFileException("", "File size is less than 13", WHERE);
	}

	if (!file.open(QIODevice::ReadOnly)) {
		throw IOException("", "Cannot open file for reading", WHERE);
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
		throw WrongFileException("", "File has incorrect signature", WHERE);
	}

	{ // Check CRC
		quint64 currentpos = buffer.pos();
		buffer.seek(fileDataArray.size() - 4);
		quint32 crc = 0;
		buffer.read(crc);

		quint32 actualCrc = crc32buf(fileDataArray.constData(), fileDataArray.size() - 4);
		if (crc != actualCrc) {
			throw WrongFileException("", "File has incorrect checksum", WHERE);
		}

		buffer.seek(currentpos);
	}
	
	quint16 r_fileVersion = 0;
	buffer.read(r_fileVersion);

	if ((r_fileVersion >> 8) > (currentFileVersion >> 8)) {
		throw WrongFileVersionException("", "", WHERE);
	}

	if (((r_fileVersion >> 8) == (currentFileVersion >> 8)) &&
		((r_fileVersion & 0x00ff) > (currentFileVersion & 0x00ff))) {
		QMessageBox::StandardButton result =
				QMessageBox::question(0, QString(),
										 "File was created in newer version of program."
										 "Some data will not be loaded. Continue?",
										 QMessageBox::Yes | QMessageBox::No);
		if (result == QMessageBox::No) {
			throw OperationAbortedException("", "", WHERE);
		}
	}

	quint8 r_compressionLevel = 0;
	buffer.read(r_compressionLevel);

	quint8 r_cipherID = 0;
	buffer.read(r_cipherID);

	if (r_cipherID != 0 && (!Cipherer().GetAvaliableCipherIDs().contains(r_cipherID))) {
		throw WrongFileVersionException("", "Cipher is not supported", WHERE);
	}

	quint8 r_hashID = 0;
	quint8 r_secureHashID = 0;

	QByteArray r_cipherKey; // correct password
	if (r_cipherID > 0) {
		Cipherer c;

		buffer.read(r_hashID); // FIXME: Check data
		if (!c.IsHashSupported(r_hashID)) {
			throw WrongFileVersionException("", "Hash algorithm is not supported", WHERE);
		}

		buffer.read(r_secureHashID); // FIXME: Check data
		if (!c.IsSecureHashSupported(r_secureHashID)) {
			throw WrongFileVersionException("", "Hash algorithm is not supported", WHERE);
		}

		quint32 passwordHashSize = 0;
		buffer.read(passwordHashSize);

		QByteArray passwordHash(passwordHashSize, 0x0);
		buffer.read(passwordHash.data(), passwordHashSize);

		bool cancel = false;

		while (true) {
			QString password = QInputDialog::getText(0, "Enter password",
												 "This document is protected. Enter password",
												 QLineEdit::Password, "", &cancel);
			if (!cancel) {
				throw OperationAbortedException("", "", WHERE);
			}

			if (!password.isEmpty()) {
				QByteArray testPasswordHash = c.GetSecureHash(password.toAscii(), r_secureHashID);
				if (passwordHash == testPasswordHash) {
					r_cipherKey = password.toAscii();
					break;
				} else {
					QMessageBox::warning(0, "Error", "Password is wrong");
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
			throw QCAException("Encryption error", "Decrypted data is empty", WHERE);
		}
	}
	if (r_compressionLevel != 0) {
		Compressor c;
		dataArray = c.Decompress(dataArray);
	}

	Document* doc = new Document();
	doc->fileVersion = r_fileVersion;
	doc->compressionLevel = r_compressionLevel;
	doc->cipherID = r_cipherID;
	doc->password = r_cipherKey;
	doc->fileName = fileName;

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
			quint32 pixmapSize = 0;
			readResult = dataBuffer.read(pixmapSize);
			QByteArray pixmapArray(pixmapSize, 0x0);
			readResult = dataBuffer.read(pixmapArray.data(), pixmapSize);
			QPixmap pixmap;
			pixmap.loadFromData(pixmapArray, iconInfo.suffix().toStdString().c_str());

			doc->customIcons.insert(QString(nameArray), pixmap);
		}
	}

	QHash<quint32, QString> iconIndexes;
	// Read icon indexes
	{
		quint32 blockSize = 0;
		readResult = dataBuffer.read(blockSize);
		const qint64 blockEndByte = dataBuffer.pos() + blockSize;

		while(dataBuffer.pos() < blockEndByte) {
			quint32 id = 0;
			readResult = dataBuffer.read(id);
			quint32 nameArraySize = 0;
			readResult = dataBuffer.read(nameArraySize);
			QByteArray nameArray(nameArraySize, 0x0);
			readResult = dataBuffer.read(nameArray.data(), nameArraySize);

			iconIndexes.insert(id, QString(nameArray));
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
			Tag* tag = Tag::Deserialize(r_fileVersion, dataBuffer);

			tagsIDs.insert(tagID, tag);
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
			Note* note = Note::Deserialize(r_fileVersion, dataBuffer);
			note->SetIcon(iconIndexes.value(note->IconIndex));

			folderItems.insert(folderItemID, note);
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
			Folder* folder = Folder::Deserialize(r_fileVersion, dataBuffer);
			folder->SetIconID(iconIndexes.value(folder->IconIndex));
			folderItems.insert(folderID, folder);
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
			Q_ASSERT(folderItems.contains(folderID));
			Folder* parentFolder = dynamic_cast<Folder*>(folderItems.value(folderID));
			Q_ASSERT(parentFolder != 0);

			quint32 childrenCount = 0;
			readResult = dataBuffer.read(childrenCount);

			while (childrenCount > 0) {
				quint32 childID = 0;
				readResult = dataBuffer.read(childID);
				Q_ASSERT(folderItems.contains(childID));
				AbstractFolderItem* childItem = folderItems.value(childID);
				parentFolder->Items.Add(childItem);
				childrenCount--;
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
			Q_ASSERT(tagsIDs.contains(tagID));
			Tag* tag = tagsIDs.value(tagID);

			quint32 ownersCount = 0;
			readResult = dataBuffer.read(ownersCount);

			while (ownersCount > 0) {
				quint32 ownerID = 0;
				readResult = dataBuffer.read(ownerID);
				Q_ASSERT(folderItems.contains(ownerID));
				Note* note = dynamic_cast<Note*>(folderItems.value(ownerID));
				Q_ASSERT(note != 0);
				note->Tags.Add(tag);
				ownersCount--;
			}
		}
	}

	// Read visual settings
	{
		quint32 activeNavigationTab = 0;
		dataBuffer.read(activeNavigationTab);
		doc->VisualSettings.ActiveNavigationTab = activeNavigationTab;

		quint32 activeNoteID = 0;
		dataBuffer.read(activeNoteID);
		if (folderItems.contains(activeNoteID)) {
			doc->VisualSettings.ActiveNote = dynamic_cast<Note*>(folderItems.value(activeNoteID));
		} else {
			doc->VisualSettings.ActiveNote = 0;
		}

		quint32 openedNotesListSize = 0;
		dataBuffer.read(openedNotesListSize);

		for (quint32 i = 0; i < openedNotesListSize; i++) {
			quint32 noteID = 0;
			quint32 position = 0;
			dataBuffer.read(noteID);
			dataBuffer.read(position);

			if (folderItems.contains(noteID)) {
				Note* note = dynamic_cast<Note*>(folderItems.value(noteID));
				doc->VisualSettings.OpenedNotes.append(QPair<Note*, int>(note, position));
			}
		}
	}

	dataBuffer.close();

	doc->isChanged = false;
	return doc;
}

void Document::Save(QString name, quint16 version) {
	if (version != 0) {
		fileVersion = version;
	}

	if (!name.isEmpty()) {
		fileName = name;
	}

	Q_ASSERT(!fileName.isEmpty());



	QByteArray fileDataArray;

	BOIBuffer fileDataBuffer(&fileDataArray);
	fileDataBuffer.open(QIODevice::WriteOnly);

	qint64 writeResult = 0;

	const char fileSignature[9] = {0x89, 0x51, 0x4E, 0x4D,
							   0x53, 0x0D, 0x0A, 0x1A, 0x0A};

	writeResult = fileDataBuffer.write(fileSignature, 9);

	writeResult = fileDataBuffer.write(fileVersion);
	writeResult = fileDataBuffer.write(compressionLevel);
	writeResult = fileDataBuffer.write(cipherID);


	QByteArray encryptionKey;
	if (cipherID > 0) {
		Cipherer c;

		quint8 r_hashID = c.DefaultHashID;
		fileDataBuffer.write(r_hashID); // FIXME: Write actual data

		quint8 r_secureHashID = c.DefaultSecureHashID;
		fileDataBuffer.write(r_secureHashID); // FIXME: Write actual data


		encryptionKey = c.GetHash(password, r_hashID);

		const QByteArray passwordHash = c.GetSecureHash(password, r_secureHashID);

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
		const quint32 docCreationDate = creationDate.toTime_t();
		const quint32 docModificationDate = modificationDate.toTime_t();
		dataBuffer.write(docCreationDate);
		dataBuffer.write(docModificationDate);

		const QByteArray defFolderIcon = DefaultFolderIcon.toAscii();
		const quint32 defFolderIconLength = defFolderIcon.length();
		dataBuffer.write(defFolderIconLength);
		dataBuffer.write(defFolderIcon.constData(), defFolderIconLength);

		const QByteArray defNoteIcon = DefaultNoteIcon.toAscii();
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

		QBuffer pixmapBuffer;

		foreach(QString name, customIcons.keys()) {
			const QByteArray nameArray = name.toUtf8();
			const quint32 nameArraySize = nameArray.size();
			QPixmap pixmap = customIcons.value(name);
			QFileInfo pixmapInfo(nameArray);
			dataBuffer.write(nameArraySize);
			dataBuffer.write(nameArray.constData(), nameArraySize);
			quint32 pixmapSize = 0;
			const qint64 pixmapSizePosition = dataBuffer.pos();
			dataBuffer.write(pixmapSize);

			QByteArray pixmapArray;
			pixmapBuffer.setBuffer(&pixmapArray);
			pixmapBuffer.open(QIODevice::WriteOnly);
			pixmap.save(&pixmapBuffer, qPrintable(pixmapInfo.suffix()));
			pixmapBuffer.close();
			dataBuffer.write(pixmapArray);

			pixmapSize = (quint32)(dataBuffer.pos() - pixmapSizePosition);
			const qint64 end = dataBuffer.pos();
			dataBuffer.seek(pixmapSizePosition);
			dataBuffer.write(pixmapSize);
			dataBuffer.seek(end);
		}
		const qint64 lastPos = dataBuffer.pos();
		iconsBlockSize = lastPos - blockStart;
		dataBuffer.seek(iconsBlockSizePosition);
		dataBuffer.write(iconsBlockSize);
		dataBuffer.seek(lastPos);
	}

	// Generate icons indexes
	QHash<QString, quint32> iconIndexes;
	{
		quint32 currentIndex = 1;
		const QList<QString>& standardIconsNames = Application::I()->GetStandardIcons().keys();
		foreach (QString name, standardIconsNames) {
			iconIndexes.insert(name, currentIndex);
			currentIndex++;
		}
		const QList<QString>& customIconsNames = customIcons.keys();
		foreach(QString name, customIconsNames) {
			iconIndexes.insert(name, currentIndex);
			currentIndex++;
		}
	}

	// Write icons indexes
	{
		quint32 blockSize = 0;
		const qint64 blockSizePosition = dataBuffer.pos();
		dataBuffer.write(blockSize);

		const qint64 blockStartPosition = dataBuffer.pos();
		foreach (const QString name, iconIndexes.keys()) {
			const quint32 id = iconIndexes.value(name);
			const QByteArray nameArray = name.toUtf8();
			const quint32 nameArraySize = nameArray.size();
			dataBuffer.write(id);
			dataBuffer.write(nameArraySize);
			dataBuffer.write(nameArray.constData(), nameArraySize);
		}
		const qint64 blockEndPosition = dataBuffer.pos();
		blockSize = blockEndPosition - blockStartPosition;
		dataBuffer.seek(blockSizePosition);
		dataBuffer.write(blockSize);
		dataBuffer.seek(blockEndPosition);
	}

	// Write tags
	QHash<const Tag*, quint32> tagsIDs;
	{
		quint32 blockSize = 0;
		const qint64 blockSizePosition = dataBuffer.pos();
		dataBuffer.write(blockSize);

		const qint64 blockStartPosition = dataBuffer.pos();
		quint32 tagID = 1;
		for (int i = 0; i < allTags.size(); ++i) {
			const Tag* tag = allTags.value(i);
			dataBuffer.write(tagID);
			tagsIDs.insert(tag, tagID);
			tagID++;
			tag->Serialize(version, dataBuffer);
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
		for (int i = 0; i < allNotes.size(); ++i) {
			Note* note = allNotes.value(i);
			dataBuffer.write(folderOrNoteID);
			folderItemsIDs.insert(note, folderOrNoteID);
			folderOrNoteID++;
			note->IconIndex = iconIndexes.value(note->GetIconID());
			note->Serialize(version, dataBuffer);
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
		folderItemsIDs.insert(rootFolder, 0);
		folderItemsIDs.insert(tempFolder, 1);
		folderItemsIDs.insert(trashFolder, 2);

		// Write user folders
		for (int i = 0; i < allFolders.size(); ++i) {
			Folder* f = allFolders.value(i);
			dataBuffer.write(folderOrNoteID);
			folderItemsIDs.insert(f, folderOrNoteID);
			folderOrNoteID++;
			f->IconIndex = iconIndexes.value(f->GetIconID());
			f->Serialize(version, dataBuffer);
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
		foldersStack.push(rootFolder);
		foldersStack.push(tempFolder);
		foldersStack.push(trashFolder);

		while (!foldersStack.isEmpty()) {
			const Folder* folder = foldersStack.pop();
			Q_ASSERT(folderItemsIDs.contains(folder));
			const quint32 folderID = folderItemsIDs.value(folder);
			const quint32 childrenCount = (quint32)folder->Items.Count();
			dataBuffer.write(folderID);
			dataBuffer.write(childrenCount);
			for (int i = 0; i < folder->Items.Count(); ++i) {
				const AbstractFolderItem* child = folder->Items.ItemAt(i);
				Q_ASSERT(folderItemsIDs.contains(child));
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
			foreach (const Tag* tag, allTags) {
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

	// Write visual settings
	{
		quint32 activeNavigationTab = VisualSettings.ActiveNavigationTab;
		dataBuffer.write(activeNavigationTab);
		quint32 activeNoteID = folderItemsIDs.value(VisualSettings.ActiveNote);
		dataBuffer.write(activeNoteID);
		quint32 openedNotesListSize = VisualSettings.OpenedNotes.size();
		dataBuffer.write(openedNotesListSize);
		for (quint32 i = 0; i < openedNotesListSize; i++) {
			quint32 noteID = folderItemsIDs.value(VisualSettings.OpenedNotes.at(i).first);
			quint32 position = VisualSettings.OpenedNotes.at(i).second;
			dataBuffer.write(noteID);
			dataBuffer.write(position);
		}
	}


	dataBuffer.close();

	if (compressionLevel > 0) {
		Compressor c;
		dataArray = c.Compress(dataArray, compressionLevel);
	}
	if (cipherID > 0) {
		Cipherer c;
		dataArray = c.Encrypt(dataArray, encryptionKey, cipherID);
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


	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly)) {
		throw IOException("", "Cannot open file for writing", WHERE);
	}

	writeResult = file.write(fileDataArray.constData(), fileDataArray.size());
	file.close();
	if (writeResult != fileDataArray.size()) {
		if (file.exists()) {
			file.remove();
		}
		throw IOException("", "Cannot write to file", WHERE);
	}

	isChanged = false;
	emit sg_Changed();
}

void Document::sl_Folder_ItemAdded(AbstractFolderItem* const item, int) {
	RegisterItem(item);
	onChange();
}

void Document::sl_Folder_ItemRemoved(AbstractFolderItem* const item) {
	UnregisterItem(item);
	onChange();
}

void Document::RegisterItem(AbstractFolderItem* const item) {
	if (item->GetItemType() == AbstractFolderItem::Type_Folder) {
		Folder* f = dynamic_cast<Folder*>(item);
		QObject::connect(f, SIGNAL(sg_ItemAdded(AbstractFolderItem*const, int)),
						 this, SLOT(sl_Folder_ItemAdded(AbstractFolderItem*const, int)));
		QObject::connect(f, SIGNAL(sg_ItemRemoved(AbstractFolderItem*const)),
						 this, SLOT(sl_Folder_ItemRemoved(AbstractFolderItem*const)));

		QObject::connect(f, SIGNAL(sg_DataChanged()), this, SLOT(sl_ItemDataChanged()));

		allFolders.append(f);
		emit sg_ItemRegistered(f);

		for (int i = 0; i < f->Items.Count(); ++i) {
			RegisterItem(f->Items.ItemAt(i));
		}

	} else if (item->GetItemType() == AbstractFolderItem::Type_Note) {
		Note* n = dynamic_cast<Note*>(item);
		QObject::connect(n, SIGNAL(sg_DataChanged()), this, SLOT(sl_ItemDataChanged()));
		QObject::connect(n, SIGNAL(sg_TagAdded(Tag*)), this, SLOT(sl_Note_TagAdded(Tag*)));
		QObject::connect(n, SIGNAL(sg_TagRemoved(Tag*)), this, SLOT(sl_Note_TagRemoved(Tag*)));

		// TODO: Register tags

		allNotes.append(n);
		emit sg_ItemRegistered(n);
	}
}

void Document::UnregisterItem(AbstractFolderItem* const item) {
	if (item->GetItemType() == AbstractFolderItem::Type_Folder) {
		Folder* f = dynamic_cast<Folder*>(item);
		QObject::disconnect(f, 0, this, 0);

		allFolders.removeAll(f);
		emit sg_ItemUnregistered(f);

		for (int i = 0; i < f->Items.Count(); ++i) {
			UnregisterItem(f->Items.ItemAt(i));
		}

	} else if (item->GetItemType() == AbstractFolderItem::Type_Note) {
		Note* n = dynamic_cast<Note*>(item);
		QObject::disconnect(n, 0, this, 0);

		n->Tags.Clear(); // ? Tags must be unregistered, but this line modifies note.

		allNotes.removeAll(n);
		emit sg_ItemUnregistered(n);
	}
}

void Document::RegisterTag(Tag* tag) {
	allTags.append(tag);
	tagsByName.insert(tag->GetName(), tag);
	QStandardItem* i = new QStandardItem(QIcon(QPixmap("./tag")), tag->GetName());
	tagsListModel->appendRow(i);
	tagsListModel->sort(0, Qt::AscendingOrder);
	emit sg_ItemRegistered(tag);
}

void Document::UnregisterTag(Tag* tag) {
	allTags.removeAll(tag);
	tagsByName.remove(tag->GetName());
	// VER: 4.2
	QList<QStandardItem *> itemsList = tagsListModel->findItems(tag->GetName(), Qt::MatchExactly, 0);
	Q_ASSERT(itemsList.size() == 1);
	// VER: 4.2
	QStandardItem * item = tagsListModel->takeItem(itemsList.at(0)->row());
	Q_ASSERT(item != 0);
	Q_ASSERT(item == itemsList.at(0));
	delete item;

	emit sg_ItemUnregistered(tag);
	delete tag; // ?
}

void Document::sl_Note_TagAdded(Tag* tag) {
	Q_ASSERT(tag != 0);

	if (!allTags.contains(tag)) {
		RegisterTag(tag);
	}
}

void Document::sl_Note_TagRemoved(Tag* tag) {
	Q_ASSERT(tag != 0);
	Q_ASSERT(allTags.contains(tag));

	if (tag->Owners.Count() == 0) {
		UnregisterTag(tag);
	}
}

// when folder or note or tag were changed
void Document::sl_ItemDataChanged() {
	onChange();
}

Tag* Document::FindTagByName(QString name) const {
	return tagsByName.contains(name) ? tagsByName.value(name) : 0;
}

QAbstractItemModel* Document::GetTagsListModel() const {
	return tagsListModel;
}

HierarchyModel* Document::GetHierarchyModel() const {
	return hierarchyModel;
}

TagsModel* Document::GetTagsModel() const {
	return tagsModel;
}

DatesModel* Document::GetCreationDatesModel() const {
	return creationDateModel;
}

DatesModel* Document::GetModificationDatesModel() const {
	return modificationDateModel;
}

DatesModel* Document::GetTextDatesModel() const {
	return textDateModel;
}

QList<Tag*> Document::GetTagsList() const {
	return allTags;
}

QList<Note*> Document::GetNotesList() const {
	return allNotes;
}

void Document::AddCustomIcon(QPixmap image, QString name) {
	QFileInfo file(name);
	QString base = file.completeBaseName();
	QString suffix = file.suffix();
	int addNumber = 1;
	while (customIcons.contains(name)) {
		name = base.append(QString::number(addNumber)).append(".").append(suffix);
		addNumber++;
	}

	customIcons.insert(name, image);

	QStandardItem* i = new QStandardItem(image, QString());
	i->setData(name, Qt::UserRole + 1);
	i->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	customIconsModel->appendRow(i);

	onChange();
}

void Document::RemoveCustomIcon(QString key) {
	if (!customIcons.contains(key)) {return;}

	QList<QStandardItem*> itemsList = customIconsModel->findItems(key, Qt::MatchExactly, 0);
	Q_ASSERT(itemsList.size() == 1);
	QStandardItem * item = customIconsModel->takeItem(itemsList.at(0)->row());
	Q_ASSERT(item != 0);
	Q_ASSERT(item == itemsList.at(0));
	for (int i = 0; i < allNotes.size(); ++i) {
		if (allNotes.at(i)->GetIconID() == key) {
			allNotes.at(i)->SetIcon(Application::I()->DefaultNoteIcon);
		}
	}

	for (int i = 0; i < allFolders.size(); ++i) {
		if (allFolders.at(i)->GetIconID() == key) {
			allFolders.at(i)->SetIconID(Application::I()->DefaultFolderIcon);
		}
	}
	delete item;
	customIcons.remove(key);

	onChange();
}

QPixmap Document::GetItemIcon(const QString key) const {
	Q_ASSERT(!key.isEmpty());

	if (key.mid(0, 1) == ":") {
		if (Application::I()->GetStandardIcons().contains(key)) {
			return Application::I()->GetStandardIcons().value(key);
		} else {
			qWarning(qPrintable("Requested non-existent icon: " + key));
			return QPixmap();
		}
	} else {
		if (customIcons.contains(key)) {
			return customIcons.value(key);
		} else {
			qWarning(qPrintable("Requested non-existent icon: " + key));
			return QPixmap();
		}
	}
}

quint8 Document::GetCompressionLevel() const {
	return compressionLevel;
}

void Document::SetCompressionLevel(const quint8 level) {
	Q_ASSERT(level >= Compressor::MinimumLevel && level <= Compressor::MaximumLevel);

	if (compressionLevel != level) {
		compressionLevel = level;
		onChange();
	}
}

quint8 Document::GetCipherID() const {
	return cipherID;
}

QString Document::GetPassword() const {
	return QString(password);
}

void Document::SetCipherData(const quint8 id, const QString& _password) {
	if (id != 0) {
		Q_ASSERT(!_password.isEmpty());
	}

	if (cipherID != id) {
		cipherID = id;
		onChange();
	}

	if (password != _password) {
		if (cipherID == 0) {
			password = QByteArray();
		} else {
			password = _password.toLocal8Bit();
		}
		onChange();
	}
}


