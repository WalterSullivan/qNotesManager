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
#include "attachedfileswidget.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

using namespace qNotesManager;

NoteEditWidget::NoteEditWidget(Note* n) : QWidget(nullptr) {
	if (n == nullptr) {
		WARNING("Null pointer recieved");
	}

	propertiesPanelCollapsed = false;

	textEditWidget = new TextEditWidget();
	QObject::connect(textEditWidget, SIGNAL(sg_LinkClicked(QUrl)),
					 this, SIGNAL(sg_LinkClicked(QUrl)));

	propertiesWidget = new QWidget();

	openClosePropertiesButton = new QPushButton();
	openClosePropertiesButton->setFocusPolicy(Qt::NoFocus);
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

	attachedFilesWidget = new AttachedFilesWidget(n);
	QObject::connect(attachedFilesWidget, SIGNAL(sg_OnResize()),
					 this, SLOT(sl_AttachFilesPanel_OnResize()));


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

	gridLayout->addWidget(attachedFilesWidget, 6, 0, 1, 5);

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
		QObject::connect(currentNote, SIGNAL(sg_PropertyChanged()),
						 this, SLOT(sl_Note_PropertyChanged()));
	}

	fillControlsWithData();
	updateControlsStatus();
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

	openClosePropertiesButton->setToolTip("Expand");
	openClosePropertiesButton->setIcon(QIcon(":/gui/arrow-down"));
	propertiesPanelCollapsed = true;

	authorEdit->setFocusPolicy(Qt::NoFocus);
	textCreationCheckbox->setFocusPolicy(Qt::NoFocus);
	textCreationDateEdit->setFocusPolicy(Qt::NoFocus);
	sourceEdit->setFocusPolicy(Qt::NoFocus);
	tagsEdit->setFocusPolicy(Qt::NoFocus);
	commentEdit->setFocusPolicy(Qt::NoFocus);
	attachedFilesWidget->SetFocusPolicyCustom(Qt::NoFocus);

	updateHeight();
}

void NoteEditWidget::expandPropertiesPanel() {
	if (!propertiesPanelCollapsed) {return;}

	openClosePropertiesButton->setToolTip("Collapse");
	openClosePropertiesButton->setIcon(QIcon(":/gui/arrow-up"));
	propertiesPanelCollapsed = false;

	authorEdit->setFocusPolicy(Qt::StrongFocus);
	textCreationCheckbox->setFocusPolicy(Qt::StrongFocus);
	textCreationDateEdit->setFocusPolicy(Qt::StrongFocus);
	sourceEdit->setFocusPolicy(Qt::StrongFocus);
	tagsEdit->setFocusPolicy(Qt::StrongFocus);
	commentEdit->setFocusPolicy(Qt::StrongFocus);
	attachedFilesWidget->SetFocusPolicyCustom(Qt::StrongFocus);

	updateHeight();
}

void NoteEditWidget::sl_AttachFilesPanel_OnResize() {
	updateHeight();
}

void NoteEditWidget::updateHeight() {
	int newHeight = 0;
	if (propertiesPanelCollapsed) {
		newHeight =
				captionEdit->height() +
				propertiesWidget->layout()->contentsMargins().top() +
				((QGridLayout*)propertiesWidget->layout())->horizontalSpacing();
	} else {
		newHeight = propertiesWidget->height();

	}
	scrollArea->setFixedHeight(newHeight);
}

Note* NoteEditWidget::CurrentNote() const {
	return currentNote;
}

void NoteEditWidget::ScrollTo(int position) {
	if (currentNote == nullptr) {
		WARNING("Null pointer recieved");
		return;
	}
	textEditWidget->ScrollTo(position);
}

void NoteEditWidget::ShowFragment(const NoteFragment& fragment) {
	if (currentNote == nullptr) {
		WARNING("Null pointer recieved");
		return;
	}

	if (fragment.NotePtr != currentNote) {
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
		WARNING("Unhandled case branch");
	}

}

int NoteEditWidget::CurrentPosition() const {
	if (currentNote == nullptr) {
		WARNING("Null pointer recieved");
		return -1;
	}
	return textEditWidget->CurrentPosition();
}

void NoteEditWidget::updateControlsData() {
	if (currentNote == nullptr) {
		WARNING("Null pointer recieved");
		setEnabled(false);
		return;
	}

	captionEdit->setText(currentNote->GetName());
	authorEdit->setText(currentNote->GetAuthor());
	textCreationCheckbox->setChecked(currentNote->GetTextCreationDate().isValid());
	textCreationDateEdit->setEnabled(!currentNoteLocked && currentNote->GetTextCreationDate().isValid());
	textCreationDateEdit->setDateTime(currentNote->GetTextCreationDate());
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

void NoteEditWidget::updateControlsStatus() {
	if (currentNote == nullptr) {
		setEnabled(false);
		currentNoteLocked = true;
		return;
	}
	if (currentNoteLocked == currentNote->IsLocked()) {
		return;
	}

	currentNoteLocked = currentNote->IsLocked();
	textEditWidget->SetReadOnly(currentNoteLocked);
	captionEdit->setEnabled(!currentNoteLocked);
	authorEdit->setEnabled(!currentNoteLocked);
	textCreationCheckbox->setEnabled(!currentNoteLocked);
	textCreationDateEdit->setEnabled(!currentNoteLocked && currentNote->GetTextCreationDate().isValid());
	sourceEdit->setEnabled(!currentNoteLocked);
	tagsEdit->setEnabled(!currentNoteLocked);
	commentEdit->setEnabled(!currentNoteLocked);
	textEditWidget->UpdateActionsStatus(!currentNoteLocked);
}

void NoteEditWidget::fillControlsWithData() {
	if (currentNote == nullptr) {
		return;
	}

	textEditWidget->SetDocument(currentNote->GetTextDocument());
	captionEdit->setText(currentNote->GetName());
	authorEdit->setText(currentNote->GetAuthor());
	if (currentNote->GetTextCreationDate().isValid()) {
		textCreationCheckbox->setChecked(true);
		textCreationDateEdit->setEnabled(true);
		textCreationDateEdit->setDateTime(currentNote->GetTextCreationDate());
	} else {
		textCreationCheckbox->setChecked(false);
		textCreationDateEdit->setEnabled(false);
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
	(void)newDateTime;
}

void NoteEditWidget::sl_TagsEdit_CollectionChanged(QStringList newTags) {
	if (currentNote == nullptr) {
		WARNING("Null pointer recieved");
		return;
	}

	for (int i = (currentNote->Tags.Count() - 1); i >= 0; i--) {
		Tag* t = currentNote->Tags.ItemAt(i);
		QString tagName = t->GetName();

		if (!newTags.contains(tagName)) {currentNote->Tags.Remove(t);}
		if (newTags.contains(tagName)) {newTags.removeAll(tagName);}
	}

	foreach (QString newTagName, newTags) {
		Tag* t = Application::I()->CurrentDocument()->FindTagByName(newTagName); // FIXME
		if (t == 0) {t = new Tag(newTagName);}
		currentNote->Tags.Add(t);
	}
}

void NoteEditWidget::sl_Note_PropertyChanged() {
	updateControlsStatus();
	updateControlsData();
}

// virtual
bool NoteEditWidget::eventFilter (QObject* watched, QEvent* event) {
	if (event->type() == QEvent::FocusOut) {
		QString text;
		if (watched == captionEdit) {
			text = captionEdit->text();
			text.replace(QRegExp("[\a\e\f\n\r\t\v]"), " ");
			if (currentNote->GetName() != text) {
				currentNote->SetName(text);
				captionEdit->setText(text);
				qDebug() << "Caption changed";
			}

		} else if (watched == authorEdit) {
			text = authorEdit->text();
			text.replace(QRegExp("[\a\e\f\n\r\t\v]"), " ");
			if (currentNote->GetAuthor() != text) {
				currentNote->SetAuthor(text);
				authorEdit->setText(text);
				qDebug() << "Author changed";
			}

		} else if (watched == sourceEdit) {
			text = sourceEdit->text();
			text.replace(QRegExp("[\a\e\f\n\r\t\v]"), " ");
			if (currentNote->GetSource() != text) {
				currentNote->SetSource(text);
				sourceEdit->setText(text);
				qDebug() << "Source changed";
			}

		} else if (watched == commentEdit) {
			text = commentEdit->text();
			text.replace(QRegExp("[\a\e\f\n\r\t\v]"), " ");
			if (currentNote->GetComment() != text) {
				currentNote->SetComment(text);
				commentEdit->setText(text);
				qDebug() << "Comment changed";
			}

		} else if (watched == textCreationCheckbox ||
				   watched == textCreationDateEdit) {
			QDateTime newDate = QDateTime();
			if (textCreationCheckbox->isChecked()) {
				newDate = textCreationDateEdit->dateTime();
			}
			if (currentNote->GetTextCreationDate() != newDate) {
				currentNote->SetTextCreationDate(textCreationDateEdit->dateTime());
			}
			qDebug() << "Text creation date changed";
		} else {
			qDebug() << "Unhandled widget focus out event";
		}
	}
	return false;
}

QList<QAction*> NoteEditWidget::EditActionsList() const {
	return textEditWidget->EditActionsList();
}

void NoteEditWidget::FocusTextEdit() {
	textEditWidget->FocusTextEdit();
}
