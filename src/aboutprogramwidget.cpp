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

#include "aboutprogramwidget.h"

#include <QGridLayout>

using namespace qNotesManager;

AboutProgramWidget::AboutProgramWidget(QWidget *parent) : QDialog(parent) {
	logoLabel = new QLabel();
	logoLabel->setPixmap(QPixmap(":/main"));

	descriptionLabel = new QLabel();
	descriptionLabel->setText(
	"qNotesManager is note taking software. Keep all your notes, important data."
	"\n\nCode by: Yury Hamenkov"
	"\nIcons:"
	"\nFugue Icons pack. Copyright (C) 2010 Yusuke Kamiyamane (p@yusukekamiyamane.com). "
	"All rights reserved."
	"\nThe icons are licensed under a Creative Commons Attribution  3.0 license. "
	"<http://creativecommons.org/licenses/by/3.0/>"
	);

	versionLabel = new QLabel();
	versionLabel->setText(QString("Current version: %1").arg(VERSION));

	okButton = new QPushButton("OK");
	QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(close()));

	QGridLayout* layout = new QGridLayout();
	layout->addWidget(logoLabel, 0, 0);
	layout->addWidget(descriptionLabel, 0, 1);
	layout->addWidget(versionLabel, 1, 1);
	layout->addWidget(okButton, 2, 1, 1, 1, Qt::AlignRight);

	setLayout(layout);
	setWindowTitle("About qNotesManager");
}


