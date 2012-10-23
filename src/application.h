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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QHash>
#include <QPixmap>

#include "applicationsettings.h"

namespace qNotesManager {
	class Document;

	class Application : public QObject {
	Q_OBJECT
	private:
		explicit Application();
		static Application* _instance;

		Document*		_currentDocument;
		QHash<QString, QPixmap> standardIcons;


	public:
		static Application* I();

		void SetCurrentDocument(Document*);
		Document* CurrentDocument() const;

		ApplicationSettings Settings;

		QHash<QString, QPixmap> GetStandardIcons() const;

		const QString DefaultNoteIcon;
		const QString DefaultFolderIcon;

	signals:
		void sg_CurrentDocumentChanged(Document* oldDoc);
	};
}

#endif // APPLICATION_H
