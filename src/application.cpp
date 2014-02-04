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

	createDummyImages();
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

void Application::createDummyImages() {
	const QSize dummyImageSize = QSize(150, 150);
	loadingDummyImage = QPixmap(dummyImageSize);
	errorDummyImage = QPixmap(dummyImageSize);
	QPainter painter;
	const QString loadingDummyImageText = "Loading image...";
	const QString errorDummyImageText = "Error loading image";
	const QBrush backgroundBrush(Qt::lightGray);
	const QRect rect(QPoint(0,0), dummyImageSize);

	painter.begin(&loadingDummyImage);
	painter.setBrush(backgroundBrush);
	painter.drawRect(rect);
	painter.drawText(rect, Qt::AlignCenter, loadingDummyImageText);
	painter.end();

	painter.begin(&errorDummyImage);
	painter.setBrush(backgroundBrush);
	painter.drawRect(rect);
	painter.drawText(rect, Qt::AlignCenter, errorDummyImageText);
	painter.end();
}

QPixmap Application::GetErrorImage() const {
	return errorDummyImage;
}

QPixmap Application::GetLoadingImage() const {
	return loadingDummyImage;
}
