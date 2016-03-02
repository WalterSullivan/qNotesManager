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
#include <QTextCodec>
#include <QFileInfo>
#include <QtGlobal>

#include <stdio.h>
#include <stdlib.h>

#include "mainwindow.h"
#include "application.h"
#include "appinfo.h"


using namespace qNotesManager;

#if QT_VERSION >= 0x050000
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
#else
void myMessageOutput(QtMsgType type, const char *msg);
#endif

bool errorOutput;

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	app.setQuitOnLastWindowClosed(false);

	QString helpScreenText = QString().append("Usage: \n").append(VER_PRODUCTNAME_STR).append(
			" [-v] [-h] [options] [file]\n"
			"-v, --version				Print version and exit.\n"
			"-h, --help				Print this screen\n"
			"Options:\n"
			"-s, --silent			Do not send data to stderr\n"
			"-fo FILE, --file-output FILE		Send debug output to file FILE\n\n"
			"file					File to open\n");
	errorOutput = true;

	QStringList arguments = QCoreApplication::arguments();

	if (arguments.count() > 1) {
		arguments.removeAt(0);

		if (arguments.count() == 1) {
			QString arg = arguments.at(0);
			if (arg == "-v" || arg == "--version") {
				fprintf(stdout, "%s version: %s\n", VER_PRODUCTNAME_STR, V_SVERSION_STR);
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
#if QT_VERSION >= 0x050000
	qInstallMessageHandler(myMessageOutput);
#else
	qInstallMsgHandler(myMessageOutput);
#endif

	MainWindow w;
	QObject::connect(&app, SIGNAL(aboutToQuit()), &w, SLOT(sl_QApplication_AboutToQuit()));


	if (Application::I()->Settings.GetShowWindowOnStart()) {
		w.show();
	}

	if (Application::I()->Settings.GetOpenLastDocumentOnStart() &&
		!Application::I()->Settings.GetLastDocumentName().isEmpty() &&
		QFileInfo(Application::I()->Settings.GetLastDocumentName()).exists()) {
		w.OpenDocument(Application::I()->Settings.GetLastDocumentName());
	}

	return app.exec();
}

#if QT_VERSION >= 0x050000
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
	QByteArray localMsg = msg.toLocal8Bit();
	switch (type) {
	case QtDebugMsg:
#ifdef DEBUG
		fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
#endif
		break;
	case QtWarningMsg:
		fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtCriticalMsg:
		fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtFatalMsg:
		fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		abort();
#if QT_VERSION >= 0x050500
	case QtInfoMsg:
		fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
#endif
	default:
		break;
	}
}
#else
void myMessageOutput(QtMsgType type, const char *msg) {
	if (!errorOutput) {
		if (type == QtFatalMsg) {
			abort();
		}
		return;
	}
	switch (type) {
	case QtDebugMsg:
#ifdef DEBUG
		fprintf(stderr, "Debug: %s\n", msg);
#endif
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
#endif
