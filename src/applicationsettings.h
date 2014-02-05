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
#include <QStringList>
#include <QSettings>

namespace qNotesManager {
	class BOIBuffer;

	class ApplicationSettings {

	public:
		ApplicationSettings();
		~ApplicationSettings();

		QPoint GetWindowPos() const;
		void SetWindowPos(const QPoint&);

		QSize GetWindowSize() const;
		void SetWindowSize(const QSize&);

		Qt::WindowStates GetWindowState() const;
		void SetWindowState(Qt::WindowStates state);

		bool GetShowNumberOfItems() const;
		void SetShowNumberOfItems(bool v);

		bool GetShowTagsTreeView() const;
		void SetShowTagsTreeView(bool v);

		bool GetShowDatesTreeView() const;
		void SetShowDatesTreeView(bool v);

		bool GetShowSystemTray() const;
		void SetShowSystemTray(bool v);

		bool GetCloseToTray() const;
		void SetCloseToTray(bool v);

		bool GetMinimizeToTray() const;
		void SetMinimizeToTray(bool v);

		bool GetMoveItemsToBin() const;
		void SetMoveItemsToBin(bool v);

		bool GetStarChangedNotes() const;
		void SetStarChangedNotes(bool v);

		bool GetCreateBackups() const;
		void SetCreateBackups(bool v);

		bool GetShowToolbar() const;
		void SetShowToolbar(bool v);

		bool GetShowStausBar() const;
		void SetShowStausBar(bool v);

		bool GetShowWindowOnStart() const;
		void SetShowWindowOnStart(bool v);

		bool GetOpenLastDocumentOnStart() const;
		void SetOpenLastDocumentOnStart(bool v);

		QString GetLastDocumentName() const;
		void SetLastDocumentName(const QString&);

		QStringList GetRecentFiles() const;
		void SetRecentFiles(const QStringList&);

	private:
		QSettings* settings;
	};
}

#endif // APPLICATIONSETTINGS_H
