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

#include "tagslineedit.h"

#include <QCompleter>
#include <QListView>
#include <QRect>
#include <QFocusEvent>
#include <QDebug>

using namespace qNotesManager;

TagsLineEdit::TagsLineEdit (QWidget *parent) : QLineEdit(parent) {
	completer = new QCompleter(this);
	QObject::connect(completer, SIGNAL(activated(QString)),
					 this, SLOT(sl_Completer_Activated(QString)));
	QListView* listView = new QListView();
	completer->setPopup(listView);
	completer->setWidget(this);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);

	connect(this, SIGNAL(textEdited(QString)),
			this, SLOT(sl_textEdited(QString)));
}

void TagsLineEdit::SetTagsModel(QAbstractItemModel* model) {
	completer->setModel(model);
}

QStringList TagsLineEdit::ParseTags() {
	QStringList tags;

	QString text = this->text();
	QString temp = "";
	int start = 0;

	while (start <= (text.length() - 1)) {
		int i = text.indexOf(";", start);
		if (i == -1) {
			if (start <= (text.length() - 1)) {
				temp = text.mid(start, text.length() - start).trimmed();
				if (!temp.isEmpty() && !tags.contains(temp, Qt::CaseInsensitive)) {
					tags.append(temp);
				}
			}
			break;
		}

		temp = text.mid(start, i - start).trimmed();
		start = i + 1;

		if (!temp.isEmpty() && !tags.contains(temp, Qt::CaseInsensitive)) {
			tags.append(temp);
		}
	}
	return tags;
}

void TagsLineEdit::editingFinished() {
	QStringList tags = ParseTags();

	emit sg_TagsCollectionChanged(tags);
}

void TagsLineEdit::sl_Completer_Activated (const QString& selectedText) {
	QString text = this->text();
	int current = cursorPosition();
	int end = current;
	int start = current == 0 ? 0 : current - 1;

	start = text.lastIndexOf(QRegExp("[ ;]"), start);
	if (start == -1) {start = 0;} else {start++;}

	end = text.indexOf(QRegExp("[ ;]"), end);
	if (end == -1) {end = text.length();} else {end--;}

	setSelection(start, end - start);
	insert(selectedText + "; ");
}

void TagsLineEdit::sl_textEdited (const QString&) {
	// Do not show completer when editing characters in the middle of a word
	if (cursorPosition() < (text().length() - 1) && (
			text().mid(cursorPosition(), 1) != " "
			||
			text().mid(cursorPosition(), 1) != ";")) {return;}

	QString text = this->text();
	int current = cursorPosition();
	int end = current;
	int start = current == 0 ? 0 : current - 1;

	start = text.lastIndexOf(QRegExp("[ ;]"), start);
	if (start == -1) {start = 0;} else {start++;}

	end = text.indexOf(QRegExp("[ ;]"), end);
	if (end == -1) {end = text.length();} else {end--;}

	if (start == end) {return;}


	QString prefix = text.mid(start, end - start).trimmed();
	if (!prefix.isEmpty()) {
		completer->setCompletionPrefix(prefix);
		completer->complete();
	}
}

/* virtual */
void TagsLineEdit::focusOutEvent (QFocusEvent * event) {
	QLineEdit::focusOutEvent(event);

	editingFinished();
}
