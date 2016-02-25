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

#include "application.h"

#include <QDir>
#include <QPainter>

using namespace qNotesManager;

/*static*/
Application* Application::I() {
	static Application instance;
	return &instance;
}

Application::Application() :
		QObject(0),
		DefaultNoteIcon(":/icons/standard/Document/document.png"),
		DefaultFolderIcon(":/icons/standard/Folder/folder.png") {
	_currentDocument = 0;
	standardIconsModel = new QStandardItemModel(this);

	LoadIconsFromDir(":/icons/standard/Document");
	LoadIconsFromDir(":/icons/standard/Folder");
	LoadIconsFromDir(":/icons/standard/Misc");
}

void Application::LoadIconsFromDir(const QString& dirName) {
	QDir dir(dirName);
	iconGroups.append(dir.dirName());
	QFileInfoList list = dir.entryInfoList();
	foreach (QFileInfo info, list) {
		QPixmap p(info.absoluteFilePath(), info.suffix().toStdString().c_str());
		standardIcons.insert(info.absoluteFilePath(), p);

		QStandardItem* iconItem = new QStandardItem(p, QString());
		iconItem->setData(info.absoluteFilePath(), Qt::UserRole + 1); // id
		iconItem->setData(dir.dirName(), Qt::UserRole + 2); // filter group
		iconItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		standardIconsModel->appendRow(iconItem);
	}
}

void Application::SetCurrentDocument(Document* doc) {
	if (doc == _currentDocument) {return;}
	Document* oldDoc = _currentDocument;

	_currentDocument = doc;
	emit sg_CurrentDocumentChanged(oldDoc);
}

Document* Application::CurrentDocument() const {
	return _currentDocument;
}

QList<QString> Application::GetStandardIconGroups()  {
	return iconGroups;
}

QStandardItemModel* Application::GetIconsModel()  {
	return standardIconsModel;
}

int Application::GetStandardIconsCount()  {
	return standardIcons.count();
}

QPixmap Application::GetStandardIcon(const QString& name)  {
	return standardIcons.value(name);
}

QPixmap Application::createImage(const QSize& size, const QString& text) const {
	QPainter painter;
	const QBrush backgroundBrush(Qt::lightGray);
	const QRect back(QPoint(0,0), size);
	const QRect border(0, 0, size.width() - 1, size.height() - 1);
	QPixmap image {size};

	painter.begin(&image);
	painter.setBrush(backgroundBrush);
	painter.setPen(Qt::NoPen);
	painter.drawRect(back);

	painter.setBrush(Qt::NoBrush);
	painter.setPen(Qt::black);
	painter.drawRect(border);

	painter.drawText(back, Qt::AlignCenter, text);
	painter.end();

	return image;
}

QPixmap Application::GetErrorImage(const QSize& size) const {
	const QSize minimalSize {150, 150};
	QSize newSize = size;
	if (!newSize.isValid()) {newSize = minimalSize;}
	if (newSize.width() < minimalSize.width()) {newSize.setWidth(minimalSize.width());}
	if (newSize.height() < minimalSize.height()) {newSize.setHeight(minimalSize.height());}


	if (!errorThumbnails.contains(newSize)) {
		QPixmap image {createImage(newSize, "Error loading image")};
		errorThumbnails.insert(newSize, image);
	}
	return errorThumbnails[newSize];
}

QPixmap Application::GetLoadingImage(const QSize& size) const {
	const QSize minimalSize {150, 150};
	QSize newSize = size;
	if (!newSize.isValid()) {newSize = minimalSize;}
	if (newSize.width() < minimalSize.width()) {newSize.setWidth(minimalSize.width());}
	if (newSize.height() < minimalSize.height()) {newSize.setHeight(minimalSize.height());}

	if (!loadingThumbnails.contains(newSize)) {
		QPixmap image {createImage(newSize, "Loading image...")};
		loadingThumbnails.insert(newSize, image);
	}
	return loadingThumbnails[newSize];
}

inline uint qHash(const QSize& size, uint seed) {
	return qHash(size.width(), seed) ^ size.height();
}
