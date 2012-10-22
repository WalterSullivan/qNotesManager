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

#include "searchwidget.h"
#include "documentsearchengine.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolTip>
#include <QDebug>

using namespace qNotesManager;

SearchWidget::SearchWidget(DocumentSearchEngine* eng, QWidget *parent) : QWidget(parent, Qt::Dialog) {
	searchLabel = new QLabel("Search text:", this);

	searchEdit = new QLineEdit(this);

	searchButton = new QPushButton("Start", this);
	QObject::connect(searchButton, SIGNAL(clicked()),
					 this, SLOT(sl_SearchButton_Clicked()));

	cancelButton = new QPushButton("Cancel", this);
	QObject::connect(cancelButton, SIGNAL(clicked()),
					 this, SLOT(sl_CancelButton_Clicked()));

	progressBar = new QProgressBar(this);
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);
	progressBar->setTextVisible(false);

	useRegexp = new QCheckBox("Use regexp", this);
	matchCase = new QCheckBox("Match case", this);
	matchWholeWord = new QCheckBox("Match whole word", this);

	QHBoxLayout* hl = new QHBoxLayout();
	hl->addWidget(searchLabel);
	hl->addWidget(searchEdit);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(searchButton);
	buttonsLayout->addWidget(cancelButton);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(hl);
	mainLayout->addWidget(useRegexp);
	mainLayout->addWidget(matchCase);
	mainLayout->addWidget(matchWholeWord);
	mainLayout->addWidget(progressBar);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);

	engine = eng;
	QObject::connect(engine, SIGNAL(sg_SearchStarted()),
					 this, SLOT(sl_SearchStarted()));
	QObject::connect(engine, SIGNAL(sg_SearchEnded()), this,
					 SLOT(sl_SearchEnded()));
	QObject::connect(engine, SIGNAL(sg_SearchProgress(int)),
					 this, SLOT(sl_SearchProgress(int)));
	QObject::connect(engine, SIGNAL(sg_SearchError(QString)),
					 this, SLOT(sl_SearchError(QString)));


	setWindowTitle("Search");
	setWindowIcon(QIcon(":/gui/magnifier"));
}

void SearchWidget::sl_SearchButton_Clicked() {
	if (engine->IsSearchActive()) {
		engine->StopSearch();
	} else {
		if (searchEdit->text().isEmpty()) {
			QToolTip::showText(searchEdit->mapToGlobal(QPoint()), "Enter search query", searchEdit);
			searchEdit->setFocus();
			return;
		}
		if (!engine->IsQueryValid(searchEdit->text(), useRegexp->isChecked())) {
			QToolTip::showText(searchEdit->mapToGlobal(QPoint()), "Search query is invalid", searchEdit);
			searchEdit->setFocus();
			return;
		}

		engine->StartSearch(searchEdit->text(), matchCase->isChecked(), matchWholeWord->isChecked(),
					   useRegexp->isChecked());
		close();
	}
}

void SearchWidget::sl_SearchStarted() {
	progressBar->setValue(0);
	searchButton->setText("Stop");
}

void SearchWidget::sl_SearchEnded() {
	progressBar->setValue(0);
	searchButton->setText("Start");
}

void SearchWidget::sl_SearchProgress(int value) {
	progressBar->setValue(value);
}

void SearchWidget::sl_SearchError(QString error) {
	if (!this->isVisible()) {this->show();}
	QToolTip::showText(searchEdit->mapToGlobal(QPoint(0, 0)), error, searchEdit);
	searchEdit->setFocus();
	searchEdit->selectAll();
}

void SearchWidget::sl_CancelButton_Clicked() {
	close();
}
