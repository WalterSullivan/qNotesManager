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

using namespace qNotesManager;

/*static*/
Application* Application::_instance = 0;

/*static*/
Application* Application::I() {
	if (Application::_instance == 0) {
		Application::_instance = new Application();
	}
	return Application::_instance;
}

Application::Application() :
		QObject(0),
		DefaultNoteIcon(":/standard/note"),
		DefaultFolderIcon(":/standard/folder") {
	_currentDocument = 0;

	QDir icons(":/standard");
	QFileInfoList list = icons.entryInfoList();
	foreach (QFileInfo info, list) {
		QPixmap p(info.absoluteFilePath(), info.suffix().toStdString().c_str());
		standardIcons.insert(info.absoluteFilePath(), p);
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

QHash<QString, QPixmap> Application::GetStandardIcons() const {
	return standardIcons;
}
