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

bool silent;

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	app.setQuitOnLastWindowClosed(false);

	QString helpScreenText = QString().append("Usage: \n").append(VER_PRODUCTNAME_STR).append(
			" [OPTIONS] [FILE]\n"
			"Options:\n"
			"-v, --version          Print version and exit\n"
			"-h, --help             Print this screen and exit\n"
			"-s, --silent           Do not send data to stderr\n"
			"FILE                   File to open\n");
	silent = false;
	QString fileToOpen = QString("");

	QStringList arguments = QCoreApplication::arguments();

	if (arguments.count() > 1) {
		arguments.removeAt(0);

		for (int i = 0; i < arguments.count(); ++i) {
			QString arg = arguments.at(i);
			if (i == arguments.count() - 1) {
				// Last argument, check if it is a file name
				QFileInfo info(arg);
				if (info.exists()) {
					fileToOpen = arg;
					continue;
				}
			}
			if (arg == "-v" || arg == "--version") {
				fprintf(stdout, "%s version: %s\n", VER_PRODUCTNAME_STR, V_SVERSION_STR);
				fflush(stdout);
				return 0;
			} else if (arg == "-h" || arg == "--help") {
				fprintf(stdout, qPrintable(helpScreenText));
				fflush(stdout);
				return 0;
			} else if (arg == "-s" || arg == "--silent") {
				silent = true;
			} else {
				fprintf(stdout, "Unrecognized option %s\n", arg.toStdString().c_str());
				fflush(stdout);
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

	if (!fileToOpen.isEmpty()) {
		w.OpenDocument(fileToOpen);
	} else if (Application::I()->Settings.GetOpenLastDocumentOnStart() &&
		!Application::I()->Settings.GetLastDocumentName().isEmpty() &&
		QFileInfo(Application::I()->Settings.GetLastDocumentName()).exists()) {
		w.OpenDocument(Application::I()->Settings.GetLastDocumentName());
	}

	return app.exec();
}

#if QT_VERSION >= 0x050000
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
	if (silent) {return;}

	QByteArray localMsg = msg.toLocal8Bit();
	switch (type) {
	case QtDebugMsg:
#ifdef DEBUG
		fprintf(stderr, "%s\n", localMsg.constData());
#endif
		break;
	case QtWarningMsg:
		fprintf(stderr, "Warning: %s\nAt %s(%u)\n", localMsg.constData(), context.file, context.line);
		break;
	case QtCriticalMsg:
		fprintf(stderr, "Critical: %s\nAt %s(%u)\n", localMsg.constData(), context.file, context.line);
		break;
	case QtFatalMsg:
		fprintf(stderr, "Fatal: %s\nAt %s(%u)\n", localMsg.constData(), context.file, context.line);
		abort();
#if QT_VERSION >= 0x050500
	case QtInfoMsg:
		fprintf(stderr, "Info: %s\nAt %s(%u)\n", localMsg.constData(), context.file, context.line);
		break;
#endif
	default:
		break;
	}
	fflush(stderr);
}
#else
void myMessageOutput(QtMsgType type, const char *msg) {
	if (silent) {return;}

	switch (type) {
	case QtDebugMsg:
#ifdef DEBUG
		fprintf(stderr, "%s\n", msg);
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
	fflush(stderr);
}
#endif
