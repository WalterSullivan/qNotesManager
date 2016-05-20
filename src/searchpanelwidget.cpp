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

#include "searchpanelwidget.h"

#include <QGridLayout>
#include <QAction>
#include <QMenu>
#include <QKeyEvent>
#include <QDebug>

#include "global.h"
#include "custommessagebox.h"

using namespace qNotesManager;

SearchPanelWidget::SearchPanelWidget(QTextEdit* edit, QWidget *parent) : QFrame(parent) {
	textEdit = edit;

	regexp.setMinimal(true);
	regexp.setCaseSensitivity(Qt::CaseInsensitive);
	regexp.setPattern("");

	setFrameShape(QFrame::Box);
	setFrameShadow(QFrame::Sunken);

	searchLineEditLabel = new QLabel("Search:");
	searchLineEdit = new QLineEdit();
	QObject::connect(searchLineEdit, SIGNAL(textChanged(QString)),
					 this, SLOT(sl_SearchTextEdit_TextChanged()));
	QObject::connect(searchLineEdit, SIGNAL(textChanged(QString)),
					 this, SLOT(sl_UpdateRegexp()));
	searchLineEdit->installEventFilter(this);

	findNextButton = new QPushButton(QIcon(":/gui/arrow"), "");
	findNextButton->setToolTip("Find next");
	findNextButton->setFlat(true);
	findNextButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
	QObject::connect(findNextButton, SIGNAL(clicked()),
					 this, SLOT(sl_FindNextButton_Pressed()));

	findPreviousButton = new QPushButton(QIcon(":/gui/arrow-180"), "");
	findPreviousButton->setToolTip("Find previous");
	findPreviousButton->setFlat(true);
	findPreviousButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
	QObject::connect(findPreviousButton, SIGNAL(clicked()),
					 this, SLOT(sl_FindPreviousButton_Pressed()));

	searchInfoLabel = new QLabel("");
	searchInfoLabel->setVisible(false);

	replaceLineEditLabel = new QLabel("Replace:");
	replaceLineEdit = new QLineEdit();
	replaceLineEdit->installEventFilter(this);
	replaceAndSearchForwardButton = new QPushButton(QIcon(":/gui/arrow"), "");
	replaceAndSearchForwardButton->setToolTip("Replace next");
	replaceAndSearchForwardButton->setFlat(true);
	replaceAndSearchForwardButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
	QObject::connect(replaceAndSearchForwardButton, SIGNAL(clicked()),
					 this, SLOT(sl_ReplaceAndSearchForwardButton_Pressed()));

	replaceAndSearchBackwardsButton = new QPushButton(QIcon(":/gui/arrow-180"), "");
	replaceAndSearchBackwardsButton->setToolTip("Replace previous");
	replaceAndSearchBackwardsButton->setFlat(true);
	replaceAndSearchBackwardsButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
	QObject::connect(replaceAndSearchBackwardsButton, SIGNAL(clicked()),
					 this, SLOT(sl_ReplaceAndSearchBackwardsButton_Pressed()));

	replaceAllButton = new QPushButton("Replace all");
	replaceAllButton->setToolTip("Replace all");
	replaceAllButton->setFlat(true);
	replaceAllButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
	QObject::connect(replaceAllButton, SIGNAL(clicked()),
					 this, SLOT(sl_ReplaceAllButton_Pressed()));

	useRegexpAction = new QAction("Use RegExp", this);
	QObject::connect(useRegexpAction, SIGNAL(toggled(bool)),
					 this, SLOT(sl_UpdateRegexp()));
	useRegexpAction->setCheckable(true);
	matchCaseAction = new QAction("Match case", this);
	QObject::connect(matchCaseAction, SIGNAL(toggled(bool)),
					 this, SLOT(sl_UpdateRegexp()));
	matchCaseAction->setCheckable(true);
	matchWholeWordAction = new QAction("Search whole word", this);
	QObject::connect(matchWholeWordAction, SIGNAL(toggled(bool)),
					 this, SLOT(sl_UpdateRegexp()));
	matchWholeWordAction->setCheckable(true);
	greedySearch = new QAction("Greedy search", this);
	QObject::connect(greedySearch, SIGNAL(toggled(bool)),
					 this, SLOT(sl_UpdateRegexp()));
	greedySearch->setCheckable(true);

	settingsMenu = new QMenu(this);
	settingsMenu->addAction(useRegexpAction);
	settingsMenu->addAction(matchCaseAction);
	settingsMenu->addAction(matchWholeWordAction);
	settingsMenu->addAction(greedySearch);

	settingsButton = new QPushButton();
	settingsButton->setMenu(settingsMenu);
	settingsButton->setIcon(QIcon(":/gui/wrench-screwdriver"));
	settingsButton->setToolTip("Search options");
	settingsButton->setFlat(true);
	settingsButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Ignored);

	QGridLayout* searchAndReplaceLayout = new QGridLayout();
#if QT_VERSION < 0x040300
	searchAndReplaceLayout->setMargin(5);
#else
	searchAndReplaceLayout->setContentsMargins(5, 5, 5, 5);
#endif
	searchAndReplaceLayout->setSpacing(2);

	searchAndReplaceLayout->addWidget(searchLineEditLabel, 0, 0);
	searchAndReplaceLayout->addWidget(searchLineEdit, 0, 1);
	searchAndReplaceLayout->addWidget(findPreviousButton, 0, 2);
	searchAndReplaceLayout->addWidget(findNextButton, 0, 3);
	searchAndReplaceLayout->addWidget(settingsButton, 0, 4);

	searchAndReplaceLayout->addWidget(searchInfoLabel, 1, 1, 1, 4);

	searchAndReplaceLayout->addWidget(replaceLineEditLabel, 2, 0);
	searchAndReplaceLayout->addWidget(replaceLineEdit, 2, 1);
	searchAndReplaceLayout->addWidget(replaceAndSearchBackwardsButton, 2, 2);
	searchAndReplaceLayout->addWidget(replaceAndSearchForwardButton, 2, 3);
	searchAndReplaceLayout->addWidget(replaceAllButton, 2, 4);

	setLayout(searchAndReplaceLayout);
	QWidget::setTabOrder(searchLineEdit, replaceLineEdit);

	sl_UpdateRegexp();
}

QString SearchPanelWidget::SearchText() const {
	return searchLineEdit->text();
}

QString SearchPanelWidget::ReplaceText() const {
	return replaceLineEdit->text();
}

bool SearchPanelWidget::eventFilter (QObject* watched, QEvent* event) {
	if (watched != searchLineEdit &&
		watched != replaceLineEdit) {
		return QObject::eventFilter(watched, event);
	}

	if (event->type() != QEvent::KeyPress) {
		return QObject::eventFilter(watched, event);
	}

	QKeyEvent* e = dynamic_cast<QKeyEvent*>(event);
	if (!e) {
		WARNING("Casting error");
		return false;
	}

	QKeySequence keySequence(e->key() + e->modifiers());

	if (watched == searchLineEdit || watched == replaceLineEdit) {
		if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
			// start search
			if (!searchLineEdit->text().isEmpty()) {
				textEdit->setFocus();
				sl_FindNext();
			}
			return true;

		}
		if (e->key() == Qt::Key_Escape ||
			keySequence == QKeySequence(QKeySequence::Find) ||
			keySequence == QKeySequence(QKeySequence::FindPrevious)) {
			// Hide search panel on Esc press
			setVisible(false);
			textEdit->setFocus();
			return true;
		}
	}

	if (keySequence == QKeySequence(QKeySequence::FindNext)) {
		sl_FindNext();
		return true;

	} else if (keySequence == QKeySequence(QKeySequence::FindPrevious)) {
		sl_FindPrevious();
		return true;
	}

	return QObject::eventFilter(watched, event);
}

std::pair<int, int> SearchPanelWidget::findNextMatch(int from, bool searchBackwards) {
	if (useRegexpAction->isChecked() && !regexp.isValid()) {
		setVisible(true);
		searchInfoLabel->setText("Regexp is invalid");
		searchInfoLabel->setVisible(true);
		searchLineEdit->setFocus();
		searchLineEdit->selectAll();
		return std::pair<int, int>(-1, -1);
	}

	if (from < 0) {from = 0;}

	int matchPos;
	if (searchBackwards) {
		matchPos = regexp.lastIndexIn(textEdit->toPlainText(), from);
	} else {
		matchPos = regexp.indexIn(textEdit->toPlainText(), from);
	}
	int matchLength = regexp.matchedLength();

	return std::pair<int, int>(matchPos, matchLength);

	// Remove selection to indicate if text was not found
	QTextCursor cursor(textEdit->document());
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 0);
	textEdit->setTextCursor(cursor);

	if (matchPos == -1) {
		if (searchLineEdit->isVisible()) {
			searchLineEdit->setFocus();
			searchLineEdit->selectAll();
			searchInfoLabel->setText("Text not found");
			searchInfoLabel->setVisible(true);
		}
	} else {
		textEdit->setFocus();
		searchInfoLabel->setText("");
		searchInfoLabel->setVisible(false);

		cursor.setPosition(matchPos);
		cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, matchLength);
		textEdit->setTextCursor(cursor);
	}
}

void SearchPanelWidget::replaceSelectedText() {
	QTextCursor cursor(textEdit->textCursor());
	if (cursor.selectionEnd() - cursor.selectionStart() == 0) {
		return;
	}

	QString replacement = replaceLineEdit->text();

	if (useRegexpAction->isChecked()) {
		for (int i = 1; i <= regexp.captureCount(); i++) {
			const QString backreference = QString("\\%1").arg(i);
			replacement.replace(backreference, regexp.cap(i));
		}
	}

	int start = cursor.selectionStart();
	int length = replacement.length();

	textEdit->insertPlainText(replacement);

	cursor.setPosition(start);
	cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, length);
	textEdit->setTextCursor(cursor);
}

void SearchPanelWidget::selectText(std::pair<int, int> selection) {
	int selectionStart = selection.first;
	int selectionLength = selection.second;

	QTextCursor cursor(textEdit->document());
	if (selectionStart == -1) {
		cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 0);
	} else {
		textEdit->setFocus();
		searchInfoLabel->setText("");
		searchInfoLabel->setVisible(false);

		cursor.setPosition(selectionStart);
		cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, selectionLength);
	}
	textEdit->setTextCursor(cursor);
}

void SearchPanelWidget::sl_SearchTextEdit_TextChanged() {
	if (searchInfoLabel->isVisible()) {
		searchInfoLabel->setText("");
		searchInfoLabel->setVisible(false);
	}
}

void SearchPanelWidget::sl_FindNextButton_Pressed() {
	sl_FindNext();
}

void SearchPanelWidget::sl_FindPreviousButton_Pressed() {
	sl_FindPrevious();
}

void SearchPanelWidget::sl_ReplaceAndSearchForwardButton_Pressed() {
	sl_ReplaceNext();
}

void SearchPanelWidget::sl_ReplaceAndSearchBackwardsButton_Pressed() {
	sl_ReplacePrevious();
}

void SearchPanelWidget::sl_ReplaceAllButton_Pressed() {
	int pos = 0;
	QTextCursor cursor;
	int counter = 0;

	while (true) {
		std::pair<int, int> result = findNextMatch(pos);

		if (result.first == -1) {break;}

		selectText(result);

		replaceSelectedText();

		counter++;

		cursor = textEdit->textCursor();
		pos = cursor.selectionEnd();
	}

	QString message = QString("%1 occurrences were replaced").arg(counter);
	CustomMessageBox msg(this, message, "Replace finished successfully", QMessageBox::Information);
	msg.show();
}

void SearchPanelWidget::sl_FindNext() {
	std::pair<int, int> result = findNextMatch(textEdit->textCursor().selectionEnd());
	selectText(result);

	if (result.first == -1) {
		if (this->isVisible()) {
			searchLineEdit->setFocus();
			searchLineEdit->selectAll();
			searchInfoLabel->setText("Text not found");
			searchInfoLabel->setVisible(true);
		}
	}
}

void SearchPanelWidget::sl_FindPrevious() {
	std::pair<int, int> result = findNextMatch(textEdit->textCursor().selectionStart() - 1, true);
	selectText(result);

	if (result.first == -1) {
		if (this->isVisible()) {
			searchLineEdit->setFocus();
			searchLineEdit->selectAll();
			searchInfoLabel->setText("Text not found");
			searchInfoLabel->setVisible(true);
		}
	}
}

void SearchPanelWidget::sl_ReplaceNext() {
	std::pair<int, int> result = findNextMatch(textEdit->textCursor().selectionEnd());
	selectText(result);

	if (result.first == -1) {
		if (this->isVisible()) {
			searchLineEdit->setFocus();
			searchLineEdit->selectAll();
			searchInfoLabel->setText("Text not found");
			searchInfoLabel->setVisible(true);
		}
	} else {
		replaceSelectedText();
	}
}

void SearchPanelWidget::sl_ReplacePrevious() {
	std::pair<int, int> result = findNextMatch(textEdit->textCursor().selectionStart() - 1, true);
	selectText(result);

	if (result.first == -1) {
		if (this->isVisible()) {
			searchLineEdit->setFocus();
			searchLineEdit->selectAll();
			searchInfoLabel->setText("Text not found");
			searchInfoLabel->setVisible(true);
		}
	} else {
		replaceSelectedText();
	}
}

void SearchPanelWidget::sl_SetSearchText(const QString& t) {
	searchLineEdit->setText(t);
}

void SearchPanelWidget::sl_SetReplaceText(const QString& t) {
	replaceLineEdit->setText(t);
}

void SearchPanelWidget::sl_SelectAllAndFocusSearchBox() {
	searchLineEdit->selectAll();
	searchLineEdit->setFocus();
}

void SearchPanelWidget::sl_SelectAllAndFocusReplaceBox() {
	replaceLineEdit->selectAll();
	replaceLineEdit->setFocus();
}

void SearchPanelWidget::sl_UpdateRegexp() {
	regexp.setMinimal(!greedySearch->isChecked());

	Qt::CaseSensitivity cs = matchCaseAction->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
	regexp.setCaseSensitivity(cs);

	QString searchText = searchLineEdit->text();

	if (!useRegexpAction->isChecked()) {
		searchText = QRegExp::escape(searchText);
	}

	if (matchWholeWordAction->isChecked()) {
		searchText.prepend("\\b");
		searchText.append("\\b");
	}

	regexp.setPattern(searchText);
}
