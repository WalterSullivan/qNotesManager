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

#include "applicationsettings.h"

using namespace qNotesManager;

ApplicationSettings::ApplicationSettings() {
	loadDefaultValues();
}

void ApplicationSettings::Load() {

}

void ApplicationSettings::Save() {

}

void ApplicationSettings::loadDefaultValues() {
	windowPosition = QPoint(0, 0);
	windowSize = QSize(600, 400);
	windowState = Qt::WindowNoState;

	preserveDocumentVisualSettings = false;
	showNumberOfItemsInParentItemTitle = true;
	showTagsTreeView = true;
	showDatesTreeView = true;
	showSystemTray = true;
	closeToTray = false;
	minimizeToTray = false;
	moveItemsToBin = false;
	showAsterixInChangedItemTitle = false;
	createBackups = false;
	showToolbar = true;
	showStausBar = true;
	ShowWindowOnStart = true;
}
