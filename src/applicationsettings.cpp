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

ApplicationSettings::ApplicationSettings() {
	settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "MyOrg", "qNotesManager");
}

ApplicationSettings::~ApplicationSettings() {
	settings->sync();
	delete settings;
}

QPoint ApplicationSettings::GetWindowPos() const {
	return settings->value("mainwindow/pos", QPoint(0, 0)).toPoint();
}

void ApplicationSettings::SetWindowPos(const QPoint& p) {
	settings->setValue("mainwindow/pos", p);
}

QSize ApplicationSettings::GetWindowSize() const {
	return settings->value("mainwindow/size", QSize(800, 600)).toSize();
}

void ApplicationSettings::SetWindowSize(const QSize& s) {
	settings->setValue("mainwindow/size", s);
}

Qt::WindowStates ApplicationSettings::GetWindowState() const {
	return (Qt::WindowStates)settings->value("mainwindow/state", Qt::WindowNoState).toInt();
}

void ApplicationSettings::SetWindowState(Qt::WindowStates state) {
	settings->setValue("mainwindow/state", (int)state);
}

bool ApplicationSettings::GetShowNumberOfItems() const {
	return settings->value("mainwindow/showitemscount", true).toBool();
}

void ApplicationSettings::SetShowNumberOfItems(bool v) {
	settings->setValue("mainwindow/showitemscount", v);
}

bool ApplicationSettings::GetShowTagsTreeView() const {
	return settings->value("mainwindow/showtagstree", true).toBool();
}

void ApplicationSettings::SetShowTagsTreeView(bool v) {
	settings->setValue("mainwindow/showtagstree", v);
}

bool ApplicationSettings::GetShowDatesTreeView() const {
	return settings->value("mainwindow/showdatestree", true).toBool();
}

void ApplicationSettings::SetShowDatesTreeView(bool v) {
	settings->setValue("mainwindow/showdatestree", v);
}

bool ApplicationSettings::GetShowSystemTray() const {
	return settings->value("app/showtrayicon", true).toBool();
}

void ApplicationSettings::SetShowSystemTray(bool v) {
	settings->setValue("app/showtrayicon", v);
}

bool ApplicationSettings::GetCloseToTray() const {
	return settings->value("app/closetotray", false).toBool();
}

void ApplicationSettings::SetCloseToTray(bool v) {
	settings->setValue("app/closetotray", v);
}

bool ApplicationSettings::GetMinimizeToTray() const {
	return settings->value("app/minimizetotray", false).toBool();
}

void ApplicationSettings::SetMinimizeToTray(bool v) {
	settings->setValue("app/minimizetotray", v);
}

bool ApplicationSettings::GetMoveItemsToBin() const {
	return settings->value("app/movetobin", true).toBool();
}

void ApplicationSettings::SetMoveItemsToBin(bool v) {
	settings->setValue("app/movetobin", v);
}

bool ApplicationSettings::GetStarChangedNotes() const {
	return settings->value("app/starchangednotes", false).toBool();
}

void ApplicationSettings::SetStarChangedNotes(bool v) {
	settings->setValue("app/starchangednotes", v);
}

bool ApplicationSettings::GetCreateBackups() const {
	return settings->value("app/createbackups", false).toBool();
}

void ApplicationSettings::SetCreateBackups(bool v) {
	settings->setValue("app/createbackups", v);
}

bool ApplicationSettings::GetShowToolbar() const {
	return settings->value("mainwindow/showtoolbar", true).toBool();
}

void ApplicationSettings::SetShowToolbar(bool v) {
	settings->setValue("mainwindow/showtoolbar", v);
}

bool ApplicationSettings::GetShowStausBar() const {
	return settings->value("mainwindow/showstatusbar", true).toBool();
}

void ApplicationSettings::SetShowStausBar(bool v) {
	settings->setValue("mainwindow/showstatusbar", v);
}

bool ApplicationSettings::GetShowWindowOnStart() const {
	return settings->value("app/showwindowonstart", true).toBool();
}

void ApplicationSettings::SetShowWindowOnStart(bool v) {
	settings->setValue("app/showwindowonstart", v);
}

bool ApplicationSettings::GetOpenLastDocumentOnStart() const {
	return settings->value("app/openlastdocument", false).toBool();
}

void ApplicationSettings::SetOpenLastDocumentOnStart(bool v) {
	settings->setValue("app/openlastdocument", v);
}

QString ApplicationSettings::GetLastDocumentName() const {
	return settings->value("app/lastdocument", QString()).toString();
}

void ApplicationSettings::SetLastDocumentName(const QString& n) {
	settings->setValue("app/lastdocument", n);
}


