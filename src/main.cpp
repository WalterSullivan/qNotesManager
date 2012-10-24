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

#include <QApplication>
#include <stdio.h>
#include <stdlib.h>

#include "mainwindow.h"

#include <QTextCodec>
#include "application.h"


using namespace qNotesManager;

void myMessageOutput(QtMsgType type, const char *msg);

bool errorOutput;

int main(int argc, char** argv) {
	QString helpScreenText = QString().append("Usage: \n").append(APPNAME).append(
			" [-v] [-h] [options] [file]\n"
			"-v, --version				Print version and exit.\n"
			"-h, --help				Print this screen\n"
			"Options:\n"
			"-s, --silent			Do not send data to stderr\n"
			"-fo FILE, --file-output FILE		Send debug output to file FILE\n\n"
			"file					File to open\n");
	errorOutput = true;

	QApplication app(argc, argv);
	app.setQuitOnLastWindowClosed(false);
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	QStringList arguments = QCoreApplication::arguments();

	if (arguments.count() > 1) {
		arguments.removeAt(0);

		if (arguments.count() == 1) {
			QString arg = arguments.at(0);
			if (arg == "-v" || arg == "--version") {
				fprintf(stdout, "%s version: 0.9.0\n", APPNAME); // FIXME: add actual version
				return 0;
			} else if (arg == "-h" || arg == "--help") {
				fprintf(stdout, qPrintable(helpScreenText));
				return 0;
			}
			if (arguments.contains("-no") || arguments.contains("--no-output")) {
				errorOutput = false;
			}
		}
	}

	qInstallMsgHandler(myMessageOutput);

	Application::I()->Settings.Load();
	MainWindow w;
	QObject::connect(&app, SIGNAL(aboutToQuit()), &w, SLOT(sl_QApplication_AboutToQuit()));
	if (Application::I()->Settings.ShowWindowOnStart) {
		w.show();
	} else {
		w.hide();
	}

	return app.exec();
}


void myMessageOutput(QtMsgType type, const char *msg) {
	if (!errorOutput) {
		if (type == QtFatalMsg) {
			abort();
		}
		return;
	}
	switch (type) {
	case QtDebugMsg:
		fprintf(stderr, "Debug: %s\n", msg);
		break;
	case QtWarningMsg:
		fprintf(stderr, "Warning: %s\n", msg);
		break;
	case QtCriticalMsg:
		fprintf(stderr, "Critical: %s\n", msg);
		break;
	case QtFatalMsg:
		fprintf(stderr, "Fatal: %s\n", msg);
		abort();
	}
}