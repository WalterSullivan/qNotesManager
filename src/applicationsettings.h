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

#ifndef APPLICATIONSETTINGS_H
#define APPLICATIONSETTINGS_H

#include <QPoint>
#include <QSize>
#include <QString>

namespace qNotesManager {
	class BOIBuffer;

	class ApplicationSettings {
	public:
		ApplicationSettings();
		void Load();
		void Save();

		QPoint windowPosition;
		QSize windowSize;
		Qt::WindowStates windowState;

		bool preserveDocumentVisualSettings;
		bool showNumberOfItemsInParentItemTitle;
		bool showTagsTreeView;
		bool showDatesTreeView;
		bool showSystemTray;
		bool closeToTray;
		bool minimizeToTray;
		bool moveItemsToBin;
		bool showAsterixInChangedItemTitle;
		bool createBackups;
		bool showToolbar;
		bool showStausBar;
		bool ShowWindowOnStart;
		bool OpenLastDocumentOnStart;
		QString LastDocumentName;

	private:
		void loadDefaultValues();
		void loadVersion_0(BOIBuffer*);
		const quint8 currentVersion;
	};
}

#endif // APPLICATIONSETTINGS_H
