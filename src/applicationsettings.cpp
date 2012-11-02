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

#include "boibuffer.h"
#include "global.h"

#include <QFile>

using namespace qNotesManager;

ApplicationSettings::ApplicationSettings() : currentVersion(0) {
	loadDefaultValues();
}

void ApplicationSettings::Load() {
	QFile f("settings");
	if (!f.exists()) {
		loadDefaultValues();
		return;
	}

	if (!f.open(QIODevice::ReadOnly)) {
		loadDefaultValues();
		WARNING("Could not open settings file for reading");
		return;
	}

	QByteArray fileDataArray(f.size(), 0x0);
	f.read(fileDataArray.data(), f.size());
	f.close();

	BOIBuffer buffer(&fileDataArray);
	buffer.open(QIODevice::ReadOnly);

	quint8 version = -1;
	buffer.read(version);

	if (version > 0) {
		WARNING("Wrong settings file version");
		loadDefaultValues();
		return;
	}

	switch (version) {
	case 0:
		loadVersion_0(&buffer);
		break;
	default:
		WARNING("Wrong settings file version");
		loadDefaultValues();
		return;
	}

	buffer.close();
}

void ApplicationSettings::loadVersion_0(BOIBuffer* buffer) {
	qint32 x = 0;
	qint32 y = 0;
	qint32 w = 0;
	qint32 h = 0;
	qint32 s = 0;


	buffer->read(x);
	buffer->read(y);

	windowPosition.setX(x);
	windowPosition.setY(y);

	buffer->read(w);
	buffer->read(h);

	windowSize.setWidth(w);
	windowSize.setHeight(h);

	buffer->read(s);
	windowState = (Qt::WindowStates)s;

	buffer->read(preserveDocumentVisualSettings);
	buffer->read(showNumberOfItemsInParentItemTitle);
	buffer->read(showTagsTreeView);
	buffer->read(showDatesTreeView);
	buffer->read(showSystemTray);
	buffer->read(closeToTray);
	buffer->read(minimizeToTray);
	buffer->read(moveItemsToBin);
	buffer->read(showAsterixInChangedItemTitle);
	buffer->read(createBackups);
	buffer->read(showToolbar);
	buffer->read(showStausBar);
	buffer->read(ShowWindowOnStart);
	buffer->read(OpenLastDocumentOnStart);
	quint32 lastDocNameSize = 0;
	buffer->read(lastDocNameSize);
	QByteArray lastDocName(lastDocNameSize, 0x0);
	buffer->read(lastDocName.data(), lastDocNameSize);
	LastDocumentName = lastDocName;
}

void ApplicationSettings::Save() {
	QByteArray fileDataArray;

	BOIBuffer buffer(&fileDataArray);
	buffer.open(QIODevice::WriteOnly);

	buffer.write(currentVersion);

	buffer.write((qint32)windowPosition.x());
	buffer.write((qint32)windowPosition.y());

	buffer.write((qint32)windowSize.width());
	buffer.write((qint32)windowSize.height());

	buffer.write((qint32)windowState);

	buffer.write(preserveDocumentVisualSettings);
	buffer.write(showNumberOfItemsInParentItemTitle);
	buffer.write(showTagsTreeView);
	buffer.write(showDatesTreeView);
	buffer.write(showSystemTray);
	buffer.write(closeToTray);
	buffer.write(minimizeToTray);
	buffer.write(moveItemsToBin);
	buffer.write(showAsterixInChangedItemTitle);
	buffer.write(createBackups);
	buffer.write(showToolbar);
	buffer.write(showStausBar);
	buffer.write(ShowWindowOnStart);
	buffer.write(OpenLastDocumentOnStart);
	QByteArray lastDocArray = LastDocumentName.toUtf8();
	buffer.write((quint32)lastDocArray.size());
	buffer.write(lastDocArray);

	buffer.close();

	QFile f("settings");

	if (!f.open(QIODevice::WriteOnly)) {
		WARNING("Could not write settings file. Check your permissions");
		return;
	}

	f.write(fileDataArray.constData(), fileDataArray.size());
	f.close();
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
	OpenLastDocumentOnStart = false;
	LastDocumentName = "";
}
