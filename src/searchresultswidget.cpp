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

#include "searchresultswidget.h"

#include "searchresultitemdelegate.h"
#include "application.h"
#include "document.h"
#include "documentsearchengine.h"
#include "searchmodelitem.h"
#include "global.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

using namespace qNotesManager;

SearchResultsWidget::SearchResultsWidget(DocumentSearchEngine* eng, QWidget *parent) :
		QWidget(parent) {

	searchResultsModel = new SearchResultsModel(this);

	treeView = new QTreeView();
	treeView->setModel(searchResultsModel);
	treeView->setItemDelegate(new SearchResultItemDelegate());
	treeView->setHeaderHidden(true);
	QObject::connect(treeView, SIGNAL(doubleClicked(const QModelIndex&)),
					 this, SLOT(sl_ListView_DoubleClicked(const QModelIndex&)));

	clearButton = new QPushButton("Clear results");
	clearButton->setToolTip("Clear search results");
	QObject::connect(clearButton, SIGNAL(clicked()),
					 this, SLOT(sl_ClearButton_Clicked()));

	expandAllButton = new QPushButton("Expand all");
	expandAllButton->setToolTip("Expand all nodes");
	QObject::connect(expandAllButton, SIGNAL(clicked()),
					 this, SLOT(sl_ExpandAllButton_Clicked()));

	stopSearchButton = new QPushButton("Stop");
	stopSearchButton->setToolTip("Stop search");
	QObject::connect(stopSearchButton, SIGNAL(clicked()),
					 this, SLOT(sl_StopSearchButton_Clicked()));

	closeButton = new QPushButton(this);
	closeButton->setIcon(QIcon(":/gui/cross-small"));
	closeButton->setToolTip("Close search results");
	closeButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	QObject::connect(closeButton, SIGNAL(clicked()),
					 this, SIGNAL(sg_CloseRequest()));

	progressBar = new QProgressBar();
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);

	engine = eng;
	QObject::connect(engine, SIGNAL(sg_SearchStarted()),
					 this, SLOT(sl_SearchStarted()));
	QObject::connect(engine, SIGNAL(sg_SearchEnded()),
					 this, SLOT(sl_SearchEnded()));
	QObject::connect(engine, SIGNAL(sg_SearchProgress(int)),
					 this, SLOT(sl_SearchProgress(int)));
	QObject::connect(engine, SIGNAL(sg_SearchResult(const NoteFragment&)),
					 this, SLOT(sl_SearchResult(const NoteFragment&)));

	QObject::connect(Application::I()->CurrentDocument(), SIGNAL(sg_ItemUnregistered(Note*)),
					 this, SLOT(sl_Document_NoteDeleted(Note*)));

	QHBoxLayout* hl = new QHBoxLayout();
#if QT_VERSION < 0x040300
	hl->setMargin(2);
#else
	hl->setContentsMargins(2, 2, 2, 2);
#endif
	hl->addWidget(clearButton);
	hl->addWidget(expandAllButton);
	hl->addWidget(stopSearchButton);
	hl->addWidget(progressBar);
	hl->addStretch();
	hl->addWidget(closeButton);

	QVBoxLayout* vl = new QVBoxLayout();
#if QT_VERSION < 0x040300
	vl->setMargin(2);
#else
	vl->setContentsMargins(2, 2, 2, 2);
#endif
	vl->addLayout(hl);
	vl->addWidget(treeView);

	stopSearchButton->setVisible(false);
	progressBar->setVisible(false);
	clearButton->setEnabled(false);
	expandAllButton->setEnabled(false);

	setLayout(vl);
}

void SearchResultsWidget::sl_SearchResult(const NoteFragment& fragment) {
	searchResultsModel->AddResult(fragment);
	clearButton->setEnabled(true);
	expandAllButton->setEnabled(true);
}

void SearchResultsWidget::sl_ClearButton_Clicked() {
	searchResultsModel->ClearResults();
	clearButton->setEnabled(false);
	expandAllButton->setEnabled(false);
}

void SearchResultsWidget::ClearResults() {
	sl_ClearButton_Clicked();
}

void SearchResultsWidget::sl_ExpandAllButton_Clicked() {
	treeView->expandAll();
}

void SearchResultsWidget::sl_StopSearchButton_Clicked() {
	engine->StopSearch();
}

void SearchResultsWidget::sl_ListView_DoubleClicked(const QModelIndex& index) {
	if (!index.isValid()) {return;}

	BaseModelItem* item = static_cast<BaseModelItem*>(index.internalPointer());
	if (item->DataType() == BaseModelItem::SearchResult) {
		SearchModelItem* si = dynamic_cast<SearchModelItem*>(item);
		if (!si) {
			WARNING("Casting error");
			return;
		}
		emit sg_ShowSearchResults(si->Fragment());
	}
}

void SearchResultsWidget::sl_SearchStarted() {
	progressBar->setValue(0);
	searchResultsModel->ClearResults();
	stopSearchButton->setVisible(true);
	progressBar->setVisible(true);
	clearButton->setEnabled(false);
	expandAllButton->setEnabled(false);

	emit sg_ShowRequest();
}

void SearchResultsWidget::sl_SearchEnded() {
	progressBar->setValue(0);
	stopSearchButton->setVisible(false);
	progressBar->setVisible(false);
}

void SearchResultsWidget::sl_SearchProgress(int value) {
	progressBar->setValue(value);
}

void SearchResultsWidget::sl_Document_NoteDeleted(Note* n) {
	if (searchResultsModel->ContainsResultForNote(n)) {
		searchResultsModel->RemoveResultsForNote(n);
	}
}
