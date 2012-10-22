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

#include "noteeditwidget.h"

#include "texteditwidget.h"
#include "note.h"
#include "tag.h"
#include "tagslineedit.h"
#include "application.h"
#include "document.h"
#include "global.h"
#include "notefragment.h"


#include <QVBoxLayout>
#include <QGridLayout>
#include <QEvent>
#include <QDebug>

using namespace qNotesManager;

NoteEditWidget::NoteEditWidget(Note* n) : QWidget(0) {
	if (n == 0) {
		WARNING("Null pointer");
	}

	propertiesPanelCollapsed = false;

	textEditWidget = new TextEditWidget();

	propertiesWidget = new QWidget();

	openClosePropertiesButton = new QPushButton();
	QObject::connect(openClosePropertiesButton, SIGNAL(clicked()),
					 this, SLOT(sl_OpenClosePropertiesButton_Clicked()));
	openClosePropertiesButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	captionLabel = new QLabel("Caption:");
	captionEdit = new QLineEdit();
	captionEdit->installEventFilter(this);

	authorLabel = new QLabel("Author:");
	authorEdit = new QLineEdit();
	authorEdit->installEventFilter(this);

	textCreationDateLabel = new QLabel("Text was created at:");
	textCreationCheckbox = new QCheckBox();
	textCreationCheckbox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	QObject::connect(textCreationCheckbox, SIGNAL(toggled(bool)),
					 this, SLOT(sl_TextCreationCheckbox_Toggled(bool)));
	textCreationCheckbox->installEventFilter(this);

	textCreationDateEdit = new QDateTimeEdit();
	QObject::connect(textCreationDateEdit, SIGNAL(dateTimeChanged(QDateTime)),
					 this, SLOT(sl_TextCreationDateTime_Changed(QDateTime)));
	textCreationDateEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	textCreationDateEdit->setEnabled(false);
	textCreationDateEdit->installEventFilter(this);

	sourceLabel = new QLabel("Text source:");
	sourceEdit = new QLineEdit();
	sourceEdit->installEventFilter(this);

	tagsLabel = new QLabel("Tags (divided by semicolon):");
	tagsEdit = new TagsLineEdit();
	tagsEdit->SetTagsModel(Application::I()->CurrentDocument()->GetTagsListModel()); // FIXME
	QObject::connect(tagsEdit, SIGNAL(sg_TagsCollectionChanged(QStringList)),
					 this, SLOT(sl_TagsEdit_CollectionChanged(QStringList)));

	commentLabel = new QLabel("Comment:");
	commentEdit = new QLineEdit();
	commentEdit->installEventFilter(this);


	// Setting up properties widget's layout
	QGridLayout* gridLayout = new QGridLayout();
	gridLayout->addWidget(captionLabel, 0, 0);
	gridLayout->addWidget(captionEdit, 0, 1, 1, 3);
	gridLayout->addWidget(openClosePropertiesButton, 0, 4);

	gridLayout->addWidget(authorLabel, 1, 0);
	gridLayout->addWidget(authorEdit, 1, 1, 1, 4);

	gridLayout->addWidget(textCreationDateLabel, 2, 0);
	gridLayout->addWidget(textCreationCheckbox, 2, 1);
	gridLayout->addWidget(textCreationDateEdit, 2, 2);
	gridLayout->addWidget(new QWidget(this), 2, 3, 1, 2);

	gridLayout->addWidget(sourceLabel, 3, 0);
	gridLayout->addWidget(sourceEdit, 3, 1, 1, 4);

	gridLayout->addWidget(tagsLabel, 4, 0);
	gridLayout->addWidget(tagsEdit, 4, 1, 1, 4);

	gridLayout->addWidget(commentLabel, 5, 0);
	gridLayout->addWidget(commentEdit, 5, 1, 1, 4);

	gridLayout->setVerticalSpacing(5);
	gridLayout->setColumnStretch(0, 0);
	gridLayout->setColumnStretch(3, 1);

	propertiesWidget->setLayout(gridLayout);
	propertiesWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	scrollArea = new QScrollArea();
	scrollArea->setWidget(propertiesWidget);
	scrollArea->setWidgetResizable(true);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setFrameStyle(QFrame::Box);
	scrollArea->setMinimumWidth(propertiesWidget->sizeHint().width());

	// Setting up main layout
	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(scrollArea);
	mainLayout->addWidget(textEditWidget);

	setLayout(mainLayout);

	sl_OpenClosePropertiesButton_Clicked();

	currentNote = n;
	if (currentNote) {
		QObject::connect(currentNote, SIGNAL(sg_DataChanged()),
						 this, SLOT(sl_Note_DataChanged()));
	}
	updateControls();
}


void NoteEditWidget::sl_OpenClosePropertiesButton_Clicked() {
	if (propertiesPanelCollapsed) {
		expandPropertiesPanel();
	} else {
		collapsePropertiesPanel();
	}
}

void NoteEditWidget::collapsePropertiesPanel() {
	if (propertiesPanelCollapsed) {return;}
	int newHeight =
			captionEdit->height() +
			propertiesWidget->layout()->contentsMargins().top() +
			propertiesWidget->layout()->contentsMargins().bottom();

	scrollArea->setFixedHeight(newHeight);

	openClosePropertiesButton->setToolTip("Expand");
	openClosePropertiesButton->setIcon(QIcon(":/gui/arrow-down"));
	propertiesPanelCollapsed = true;
}

void NoteEditWidget::expandPropertiesPanel() {
	if (!propertiesPanelCollapsed) {return;}

	int newHeight = propertiesWidget->height();
	scrollArea->setFixedHeight(newHeight);
	openClosePropertiesButton->setToolTip("Collapse");
	openClosePropertiesButton->setIcon(QIcon(":/gui/arrow-up"));
	propertiesPanelCollapsed = false;
}

Note* NoteEditWidget::CurrentNote() const {
	return currentNote;
}

void NoteEditWidget::ScrollTo(int position) {
	if (currentNote == 0) {
		WARNING("Null pointer");
		return;
	}
	textEditWidget->ScrollTo(position);
}

void NoteEditWidget::ShowFragment(const NoteFragment& fragment) {
	if (currentNote == 0) {
		WARNING("Null pointer");
		return;
	}

	if (fragment.NotePrt != currentNote) {
		WARNING("Wrong fragment");
		return;
	}

	switch (fragment.Type) {
	case NoteFragment::TextFragment:
		textEditWidget->ShowFragment(fragment);
		break;
	case NoteFragment::CaptionFragment:
		expandPropertiesPanel();
		captionEdit->setFocus();
		captionEdit->setSelection(fragment.MatchStart, fragment.MatchLength);
		break;
	case NoteFragment::AuthorFragment:
		expandPropertiesPanel();
		authorEdit->setFocus();
		authorEdit->setSelection(fragment.MatchStart, fragment.MatchLength);
		break;
	case NoteFragment::CommentFragment:
		expandPropertiesPanel();
		commentEdit->setFocus();
		commentEdit->setSelection(fragment.MatchStart, fragment.MatchLength);
		break;
	case NoteFragment::SourceFragment:
		expandPropertiesPanel();
		sourceEdit->setFocus();
		sourceEdit->setSelection(fragment.MatchStart, fragment.MatchLength);
		break;
	default:
		qDebug() << "Unhandled case branch!";
	}

}

int NoteEditWidget::CurrentPosition() const {
	if (currentNote == 0) {
		WARNING("Null pointer");
		return -1;
	}
	return textEditWidget->CurrentPosition();
}

void NoteEditWidget::updateControls() {
	if (currentNote == 0) {
		WARNING("Null pointer");
		setEnabled(false);
		return;
	}

	setEnabled(!currentNote->IsLocked());

	textEditWidget->SetDocument(currentNote->GetTextDocument());
	captionEdit->setText(currentNote->GetName());
	authorEdit->setText(currentNote->GetAuthor());
	if (currentNote->GetTextCreationDate().isValid()) {
		textCreationCheckbox->setChecked(true);
		textCreationDateEdit->setDateTime(currentNote->GetTextCreationDate());
	} else {
		textCreationCheckbox->setChecked(false);
		textCreationDateEdit->setDateTime(currentNote->GetTextCreationDate());
	}

	sourceEdit->setText(currentNote->GetSource());
	QString tagsList;
	for (int i = 0; i < currentNote->Tags.Count(); ++i) {
		const Tag* const tag = currentNote->Tags.ItemAt(i);

		if (i > 0) {tagsList.append("; ");}
		tagsList.append(tag->GetName());
	}
	tagsEdit->setText(tagsList);
	commentEdit->setText(currentNote->GetComment());
}

void NoteEditWidget::sl_TextCreationCheckbox_Toggled(bool toggle) {
	textCreationDateEdit->setEnabled(toggle);
}

void NoteEditWidget::sl_TextCreationDateTime_Changed(const QDateTime& newDateTime) {

}

void NoteEditWidget::sl_TagsEdit_CollectionChanged(QStringList tags) {
	qDebug() << "Tags collection changed";

	if (currentNote == 0) {
		WARNING("Null pointer");
		return;
	}

	for (int i = (currentNote->Tags.Count() - 1); i >= 0; i--) {
		Tag* t = currentNote->Tags.ItemAt(i);
		if (!tags.contains(t->GetName())) {currentNote->Tags.Remove(t);}
		if (tags.contains(t->GetName())) {tags.removeAll(t->GetName());}
	}
	foreach (QString s, tags) {
		Tag* t = Application::I()->CurrentDocument()->FindTagByName(s); // FIXME
		if (t == 0) {t = new Tag(s);}
		currentNote->Tags.Add(t);
	}
}

void NoteEditWidget::sl_Note_DataChanged() {
	updateControls();
}

// virtual
bool NoteEditWidget::eventFilter (QObject* watched, QEvent* event) {
	if (event->type() != QEvent::FocusOut) {return false;}

	if (watched == captionEdit) {
		currentNote->SetName(captionEdit->text());
		qDebug() << "Caption changed";

	} else if (watched == authorEdit) {
		currentNote->SetAuthor(authorEdit->text());
		qDebug() << "Author changed";

	} else if (watched == sourceEdit) {
		currentNote->SetSource(sourceEdit->text());
		qDebug() << "Source changed";

	} else if (watched == commentEdit) {
		currentNote->SetComment(commentEdit->text());
		qDebug() << "Comment changed";

	} else if (watched == textCreationCheckbox ||
			   watched == textCreationDateEdit) {
		if (!textCreationCheckbox->isChecked()) {
			currentNote->SetTextCreationDate(QDateTime());
		} else {
			currentNote->SetTextCreationDate(textCreationDateEdit->dateTime());
		}
		qDebug() << "Text creation date changed";
	} else {
		qDebug() << "Unhandled widget focus out event";
	}

	return false;
}

QList<QAction*> NoteEditWidget::EditActionsList() const {
	return textEditWidget->EditActionsList();
}
