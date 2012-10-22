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

#include <QDate>
#include <QDebug>
#include <QList>
#include <QColor>
#include <QString>
#include <QBuffer>
#include <QFile>
#include <QTextCodec>
#include "tracelogger.h"
#include "folder.h"
#include "note.h"
#include "document.h"
#include "application.h"
#include "tag.h"

#include "cipherer.h"


using namespace qNotesManager;

void myMessageOutput(QtMsgType type, const char *msg);

bool errorOutput;

int main(int argc, char** argv) {
	QString helpScreenText = QString().append("Usage: \n").append(APPNAME).append(
			" [-v] [-h] [options] [file]\n"
			"-v, --version				Print version and exit.\n"
			"-h, --help				Print this screen\n"
			"Options:\n"
			"-d, --hidden				Start program hidden\n"
			"-s, --silent			Do not send data to stderr\n"
			"-fo FILE, --file-output FILE		Send debug output to file FILE\n\n"
			"file					File to open\n");
	errorOutput = true;

	QApplication app(argc, argv);
	app.setQuitOnLastWindowClosed(false);
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	QStringList arguments = QCoreApplication::arguments();

	bool showMainWindow = true;



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
			if (arguments.contains("-d") || arguments.contains("--hidden")) {
				showMainWindow = false;
			}
		}


	}

	qInstallMsgHandler(myMessageOutput);

	{
		Note* n = new Note("");
		Tag* t = new Tag("1");
		n->Tags.Add(t);
		t = new Tag("2");
		n->Tags.Add(t);
		n->Tags.Clear();

		delete n;
	}

	Document* doc = new Document();

	{
		Folder* f = new Folder("Programming");
		Note* n1 = new Note("C++");
		f->Items.Add(n1);
		Note* n2 = new Note("Tips & tricks");
		f->Items.Add(n2);
		Note* n3 = new Note("C#");
		f->Items.Add(n3);

		doc->GetRoot()->Items.Add(f);

		f = new Folder("Movies");
		Note* n4 = new Note("To Watch");
		f->Items.Add(n4);
		Folder* f2 = new Folder("Revies");
		Note* n5 = new Note("Review1");
		f2->Items.Add(n5);
		f->Items.Add(f2);

		doc->GetRoot()->Items.Add(f);

		Tag* t1 = new Tag("C");
		Tag* t2 = new Tag("Cool");
		Tag* t3 = new Tag("Read later");
		Tag* t4 = new Tag("Important");

		n1->Tags.Add(t1);
		n2->Tags.Add(t1);
		n2->Tags.Add(t4);
		n4->Tags.Add(t2);
		n5->Tags.Add(t3);
		n3->Tags.Add(t2);


	}





	MainWindow w;
	if (showMainWindow) {
		w.show();
	}


Application::I()->SetCurrentDocument(doc);

	return app.exec();


	return 0;
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
