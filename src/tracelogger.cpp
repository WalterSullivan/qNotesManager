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

#include "tracelogger.h"
#include <QDebug>
#include <QString>

using namespace qNotesManager;

int TraceLogger::Indent = 0;

TraceLogger::TraceLogger(const char* fileName, const char* funcName, int lineNumber) {
	_fileName = fileName;
	_funcName = funcName;
	qDebug("%sEntering  %s() - (%s : %d)", QString(TraceLogger::Indent, ' ').toStdString().c_str(),
		   _funcName, _fileName, lineNumber);
	Indent += indentStep;
}

TraceLogger::~TraceLogger() {
	Indent -= indentStep;
	qDebug("%sLeaving  %s() - (%s)", QString(TraceLogger::Indent, ' ').toStdString().c_str(), _funcName,
		   _fileName);
}

void TraceLogger::stage(const char* stage) {
	qDebug("%s %s() - STAGE %s", QString(TraceLogger::Indent, ' ').toStdString().c_str(), _funcName,
		   stage);
}
