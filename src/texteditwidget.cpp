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

#include "texteditwidget.h"

#include "textedit.h"
#include "textdocument.h"
#include "colorpickerbutton.h"
#include "global.h"

#include <QVBoxLayout>
#include <QAction>
#include <QFontComboBox>
#include <QComboBox>
#include <QTextBlock>
#include <QTextList>
#include <QKeyEvent>
#include <QRegExp>
#include <QToolTip>
#include <QTextTable>
#include <QDebug>

using namespace qNotesManager;

TextEditWidget::TextEditWidget(QWidget *parent) : QWidget(parent) {
	CreateControls();
	readOnly = false;
}

void TextEditWidget::CreateControls() {
	// Create TextEdit
	textField = new TextEdit();
	QFont c = textField->currentFont();
	c.setPointSize(9);
	textField->setCurrentFont(c);
	textField->installEventFilter(this);

	QObject::connect(textField, SIGNAL(textChanged()),
					 this, SIGNAL(sg_TextChanged()));
	QObject::connect(textField, SIGNAL(textChanged()),
					 this, SLOT(sl_TextEdit_CursorPositionChanged()));
	QObject::connect(textField, SIGNAL(selectionChanged()),
					 this, SLOT(sl_TextEdit_SelectionChanged()));
	QObject::connect(textField, SIGNAL(cursorPositionChanged()),
					 this, SLOT(sl_TextEdit_CursorPositionChanged()));
	QObject::connect(textField, SIGNAL(undoAvailable(bool)),
					 this, SLOT(sl_TextEdit_UndoAvaliable(bool)));
	QObject::connect(textField, SIGNAL(redoAvailable(bool)),
					 this, SLOT(sl_TextEdit_RedoAvaliable(bool)));



	// Create toolbar
	TBRMainBar = new QToolBar();

	fontComboBox = new QFontComboBox();
	QObject::connect(fontComboBox,SIGNAL(currentFontChanged(QFont)),
					 this, SLOT(sl_fontComboBoxCurrentFontChanged(QFont)) );
	TBRMainBar->addWidget(fontComboBox);

	fontSizeComboBox = new QComboBox();
	fontSizeComboBox->addItem("8", 8);
	fontSizeComboBox->addItem("9", 9);
	fontSizeComboBox->addItem("10", 10);
	fontSizeComboBox->addItem("11", 11);
	fontSizeComboBox->addItem("12", 12);
	fontSizeComboBox->addItem("13", 13);
	fontSizeComboBox->addItem("14", 14);
	fontSizeComboBox->addItem("15", 15);
	fontSizeComboBox->addItem("16", 16);
	fontSizeComboBox->addItem("17", 17);
	fontSizeComboBox->addItem("18", 18);
	fontSizeComboBox->addItem("19", 19);
	fontSizeComboBox->addItem("20", 20);
	fontSizeComboBox->addItem("22", 22);
	QObject::connect(fontSizeComboBox, SIGNAL(currentIndexChanged(int)),
					 this, SLOT(sl_fontSizeComboBoxCurrentIndexChanged(int)));
	TBRMainBar->addWidget(fontSizeComboBox);

	TBRMainBar->addSeparator();

	ACTClearFormatting = TBRMainBar->addAction(QIcon(":/gui/eraser"), "Clear formatting");
	QObject::connect(ACTClearFormatting, SIGNAL(triggered()),
					 this, SLOT(sl_ClearFormattingAction_Clicked()));

	TBRMainBar->addSeparator();

	copyFormatAction = TBRMainBar->addAction(QIcon(":/gui/broom"), "Copy format");
	copyFormatAction->setCheckable(true);
	QObject::connect(copyFormatAction, SIGNAL(triggered(bool)),
					 textField, SLOT(sl_CopyCurrentFormat(bool)));
	QObject::connect(textField, SIGNAL(sg_CopyFormatCleared(bool)),
					 copyFormatAction, SLOT(setChecked(bool)));

	TBRMainBar->addSeparator();

	ACTBold = TBRMainBar->addAction(QIcon(":/gui/edit-bold"), "Bold");
	ACTBold->setCheckable(true);
	QObject::connect(ACTBold, SIGNAL(triggered(bool)),
					 this, SLOT(sl_BoldAction_Triggered(bool)));

	ACTItalic = TBRMainBar->addAction(QIcon(":/gui/edit-italic"), "Italic");
	ACTItalic->setCheckable(true);
	QObject::connect(ACTItalic, SIGNAL(triggered(bool)),
					 this, SLOT(sl_ItalicAction_Triggered(bool)));

	ACTUnderline = TBRMainBar->addAction(QIcon(":/gui/edit-underline"), "Underline");
	ACTUnderline->setCheckable(true);
	QObject::connect(ACTUnderline, SIGNAL(triggered(bool)),
					 this, SLOT(sl_UnderlineAction_Triggered(bool)));

	strikeOutAction = TBRMainBar->addAction(QIcon(":/gui/edit-strike"), "Strike out");
	strikeOutAction->setCheckable(true);
	QObject::connect(strikeOutAction, SIGNAL(triggered(bool)),
					 this, SLOT(sl_StrikeoutAction_Triggered(bool)));

	TBRMainBar->addSeparator();

	ACTAlignLeft = TBRMainBar->addAction(QIcon(":/gui/edit-alignment"), "Align Left");
	ACTAlignLeft->setCheckable(true);
	QObject::connect(ACTAlignLeft, SIGNAL(triggered(bool)),
					 this, SLOT(sl_AlignLeftAction_Triggered(bool)));

	ACTAlignCenter = TBRMainBar->addAction(QIcon(":/gui/edit-alignment-center"), "Align Center");
	ACTAlignCenter->setCheckable(true);
	QObject::connect(ACTAlignCenter, SIGNAL(triggered(bool)),
					 this, SLOT(sl_AlignCenterAction_Triggered(bool)));

	ACTAlignRight = TBRMainBar->addAction(QIcon(":/gui/edit-alignment-right"), "Align Right");
	ACTAlignRight->setCheckable(true);
	QObject::connect(ACTAlignRight, SIGNAL(triggered(bool)),
					 this, SLOT(sl_AlignRightAction_Triggered(bool)));

	alignJustifyAction = TBRMainBar->addAction(QIcon(":/gui/edit-alignment-justify"), "Align justify");
	alignJustifyAction->setCheckable(true);
	QObject::connect(alignJustifyAction, SIGNAL(triggered(bool)),
					 this, SLOT(sl_AlignJustifyAction_Triggered(bool)));

	TBRMainBar->addSeparator();

	listButton = new QToolButton();
	listButton->setText("List");
	listButton->setFocusPolicy(Qt::NoFocus);
	listButton->setPopupMode(QToolButton::MenuButtonPopup);
	listButton->setCheckable(true);
	listButton->setIcon(QIcon(":/gui/edit-list"));
	QMenu *listMenu = new QMenu(this);
	{
		QAction* a = listMenu->addAction("Filled circle");
		a->setData(QTextListFormat::ListDisc);
		a = listMenu->addAction("Empty circle");
		a->setData(QTextListFormat::ListCircle);
		a = listMenu->addAction("Filled square");
		a->setData(QTextListFormat::ListSquare);
		listButton->setMenu(listMenu);
	}
	QObject::connect(listButton, SIGNAL(triggered(QAction*)),
					 this, SLOT(sl_ListButton_Triggered(QAction*)));
	QObject::connect(listButton, SIGNAL(toggled(bool)),
					 this, SLOT(sl_ListButton_Toggled(bool)));

	TBRMainBar->addWidget(listButton);

	increaseListIndentAction = TBRMainBar->addAction(QIcon(":/gui/arrow"), "Move right");
	QObject::connect(increaseListIndentAction, SIGNAL(triggered()),
					 this, SLOT(sl_IncreaseListIndent_Triggered()));
	increaseListIndentAction->setVisible(false);

	decreaseListIndentAction = TBRMainBar->addAction(QIcon(":/gui/arrow-180"), "Move left");
	QObject::connect(decreaseListIndentAction, SIGNAL(triggered()),
					 this, SLOT(sl_DecreaseListIndent_Triggered()));
	decreaseListIndentAction->setVisible(false);

	TBRMainBar->addSeparator();

	createTableAction = TBRMainBar->addAction(QIcon(":/gui/table"), "Create table");
	QObject::connect(createTableAction, SIGNAL(triggered()),
					 this, SLOT(sl_CreateTableAction_Triggered()));

	insertRowAction = TBRMainBar->addAction(QIcon(":/gui/table-insert-row"), "Insert row");
	QObject::connect(insertRowAction, SIGNAL(triggered()),
					 this, SLOT(sl_InsertRowAction_Triggered()));
	insertRowAction->setVisible(false);

	insertColumnAction = TBRMainBar->addAction(QIcon(":/gui/table-insert-column"), "Insert column");
	QObject::connect(insertColumnAction, SIGNAL(triggered()),
					 this, SLOT(sl_InsertColumnAction_Triggered()));
	insertColumnAction->setVisible(false);

	removeRowAction = TBRMainBar->addAction(QIcon(":/gui/table-delete-row"), "Remove row");
	QObject::connect(removeRowAction, SIGNAL(triggered()),
					 this, SLOT(sl_RemoveRowAction_Triggered()));
	removeRowAction->setVisible(false);

	removeColumnAction = TBRMainBar->addAction(QIcon(":/gui/table-delete-column"), "Remove column");
	QObject::connect(removeColumnAction, SIGNAL(triggered()),
					 this, SLOT(sl_RemoveColumnAction_Triggered()));
	removeColumnAction->setVisible(false);

	mergeCellsAction = TBRMainBar->addAction(QIcon(":/gui/table-join"), "Merge cells");
	QObject::connect(mergeCellsAction, SIGNAL(triggered()),
					 this, SLOT(sl_MergeCellsAction_Triggered()));
	mergeCellsAction->setVisible(false);

	TBRMainBar->addSeparator();

	foregroundTextColorButton = new ColorPickerButton(ColorPickerButton::TextForegroundColor);
	QObject::connect(foregroundTextColorButton, SIGNAL(clicked()),
					 this, SLOT(sl_ForegroundTextColorButton_Clicked()));
	foregroundTextColorButton->setToolTip(tr("Text color"));
	foregroundTextColorButton->setIcon(QIcon(":/gui/text-color"));
	TBRMainBar->addWidget(foregroundTextColorButton);

	backgroundTextColorButton = new ColorPickerButton(ColorPickerButton::TextBackgroundColor);
	QObject::connect(backgroundTextColorButton, SIGNAL(clicked()),
					 this, SLOT(sl_BackgroundTextColorButton_Clicked()));
	backgroundTextColorButton->setToolTip(tr("Text background color"));
	backgroundTextColorButton->setIcon(QIcon(":/gui/background-color"));
	TBRMainBar->addWidget(backgroundTextColorButton);

	textUnderlineColorButton = new ColorPickerButton(ColorPickerButton::TextUnderlineColor);
	QObject::connect(textUnderlineColorButton, SIGNAL(clicked()),
					 this, SLOT(sl_TextUnderlineColorButton_Clicked()));
	textUnderlineColorButton->setToolTip("Text underline color");
	textUnderlineColorButton->setIcon(QIcon(":/gui/underline-color"));
	TBRMainBar->addWidget(textUnderlineColorButton);

	undoAction = new QAction(QIcon(":/gui/undo"), "Undo", this);
	undoAction->setShortcut(QKeySequence::Undo);
	undoAction->setEnabled(false);
	QObject::connect(undoAction, SIGNAL(triggered()), textField, SLOT(undo()));

	redoAction = new QAction(QIcon(":/gui/redo"), "Redo", this);
	redoAction->setShortcut(QKeySequence::Redo);
	redoAction->setEnabled(false);
	QObject::connect(redoAction, SIGNAL(triggered()), textField, SLOT(redo()));


	QAction* separator = new QAction(this);
	separator->setSeparator(true);

	publicActionsList.append(undoAction);
	publicActionsList.append(redoAction);

	publicActionsList.append(separator);

	publicActionsList.append(ACTClearFormatting);
	publicActionsList.append(copyFormatAction);
	publicActionsList.append(ACTBold);
	publicActionsList.append(ACTItalic);
	publicActionsList.append(ACTUnderline);
	publicActionsList.append(strikeOutAction);

	QActionGroup* group = new QActionGroup(this);
	group->addAction(ACTAlignLeft);
	group->addAction(ACTAlignCenter);
	group->addAction(ACTAlignRight);
	group->addAction(alignJustifyAction);

	separator = new QAction(this);
	separator->setSeparator(true);
	separator->setText("Alignment");
	publicActionsList.append(separator);

	publicActionsList.append(ACTAlignLeft);
	publicActionsList.append(ACTAlignCenter);
	publicActionsList.append(ACTAlignRight);
	publicActionsList.append(alignJustifyAction);

	separator = new QAction(this);
	separator->setSeparator(true);
	separator->setText("Insert");
	publicActionsList.append(separator);

	publicActionsList.append(textField->InsertHyperlinkAction);
	publicActionsList.append(textField->InsertImageFromFileAction);
	publicActionsList.append(textField->InsertImageFromUrlAction);
	publicActionsList.append(textField->InsertPlainTextAction);
	publicActionsList.append(textField->InsertLineAction);
	publicActionsList.append(textField->InsertDateTimeAction);

	// Create search widget
	searchFrame = new QFrame();
	searchFrame->setFrameShape(QFrame::Box);
	searchFrame->setFrameShadow(QFrame::Sunken);
	searchEdit = new QLineEdit();
	searchEdit->installEventFilter(this);
	searchRegex = new QCheckBox("Use regex");
	searchMatchCase = new QCheckBox("Match case");
	searchWholeWord = new QCheckBox("Search whole word");
	QHBoxLayout* searchl = new QHBoxLayout();
#if QT_VERSION < 0x040300
	searchl->setMargin(2);
#else
	searchl->setContentsMargins(2, 2, 2, 2);
#endif
	searchl->addWidget(searchEdit);
	searchl->addWidget(searchRegex);
	searchl->addWidget(searchMatchCase);
	searchl->addWidget(searchWholeWord);
	searchFrame->setLayout(searchl);

	// Create layout
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(TBRMainBar);
	layout->addWidget(textField);
	layout->addWidget(searchFrame);
	searchFrame->setVisible(false);
	layout->setSpacing(3);
	layout->setContentsMargins(1, 1, 1, 1);
	setLayout(layout);

	textField->setFocus();
}

QString TextEditWidget::toPlainText() const {
	return this->textField->toPlainText();
}

QString TextEditWidget::toHtml() const {
	return this->textField->toHtml();
}

void TextEditWidget::setHtml (const QString& text) {
	textField->setHtml(text);
}

void TextEditWidget::setPlainText (const QString& text) {
	textField->setPlainText(text);
}

// test function
QVariant TextEditWidget::GetResource(QString id) {
	return textField->document()->resource(QTextDocument::ImageResource, QUrl(id));
}

// test function
void TextEditWidget::SetResource (QString id, QVariant resource) {
	// you can replace resource data (for specific url) this way
	textField->document()->addResource(QTextDocument::ImageResource, QUrl(id), resource);
	textField->setLineWrapColumnOrWidth ( 0 );
}

void TextEditWidget::SetReadOnly(bool ro) {
	if (readOnly == ro) {return;}

	readOnly = ro;

	TBRMainBar->setEnabled(!readOnly);
	textField->setReadOnly(readOnly);
}

// Event handlers ======================================================================================

void TextEditWidget::sl_ClearFormattingAction_Clicked() {
	textField->ClearFormatting();
}

void TextEditWidget::sl_ForegroundTextColorButton_Clicked() {
	textField->SetSelectionForeColor(foregroundTextColorButton->GetCurrentColor());
}

void TextEditWidget::sl_BackgroundTextColorButton_Clicked() {
	textField->SetSelectionBackColor(backgroundTextColorButton->GetCurrentColor());
}

void TextEditWidget::sl_TextUnderlineColorButton_Clicked() {
	textField->SetSelectionUnderlineColor(textUnderlineColorButton->GetCurrentColor());
}

void TextEditWidget::sl_BoldAction_Triggered(bool toggle) {
	textField->SetSelectionBold(toggle);
}

void TextEditWidget::sl_ItalicAction_Triggered(bool toggle) {
	this->textField->SetSelectionItalic(toggle);
}

void TextEditWidget::sl_UnderlineAction_Triggered(bool toggle) {
	this->textField->SetSelectionUnderline(toggle);
}

void TextEditWidget::sl_StrikeoutAction_Triggered (bool toggle) {
	textField->SetSelectionStrikeOut(toggle);
}

void TextEditWidget::sl_AlignLeftAction_Triggered(bool){
	textField->SetAlignment(Qt::AlignLeft);
}

void TextEditWidget::sl_AlignCenterAction_Triggered(bool toggle){
	if (toggle) {
		textField->SetAlignment(Qt::AlignHCenter);
	} else {
		textField->SetAlignment(Qt::AlignLeft);
	}
}

void TextEditWidget::sl_AlignRightAction_Triggered(bool toggle) {
	if (toggle) {
		textField->SetAlignment(Qt::AlignRight);
	} else {
		textField->SetAlignment(Qt::AlignLeft);
	}
}

void TextEditWidget::sl_AlignJustifyAction_Triggered(bool toggle) {
	if (toggle) {
		textField->SetAlignment(Qt::AlignJustify);
	} else {
		textField->SetAlignment(Qt::AlignLeft);
	}
}

void TextEditWidget::sl_ListButton_Toggled(bool toggle) {
	QTextCursor cursor = this->textField->textCursor();
	if (toggle) {
		if (cursor.currentList()) {
			WARNING("Wrong button state");
			return;
		}
		QTextListFormat format;
		format.setStyle(QTextListFormat::ListDisc);
		cursor.createList(format);
	} else {
		QTextList *textList = cursor.currentList();
		if (!cursor.currentList()) {
			WARNING("Wrong button state");
			return;
		}
		QTextBlock block = cursor.block();
		textList->remove(block);
		QTextBlockFormat format = block.blockFormat();
		format.setIndent(0);
		cursor.setBlockFormat(format);
	}
}

void TextEditWidget::sl_ListButton_Triggered(QAction* action) {
	if (!action) {
		WARNING("Null pointer recieved");
		return;
	}
	QTextListFormat::Style style = (QTextListFormat::Style)action->data().toInt();

	QTextCursor cursor = this->textField->textCursor();
	QTextList *textList = cursor.currentList();
	if (!textList) {
		WARNING("Wrong button state");
		return;
	}
	QTextListFormat format = textList->format();
	format.setStyle(style);
	textList->setFormat(format);
}

void TextEditWidget::sl_IncreaseListIndent_Triggered() {
	QTextCursor cursor = this->textField->textCursor();
	QTextList *textList = cursor.currentList();
	if (!textList) {return;}

	QTextListFormat format = textList->format();
	format.setIndent(format.indent() + 1);
	textList->setFormat(format);
}

void TextEditWidget::sl_DecreaseListIndent_Triggered() {
	QTextCursor cursor = this->textField->textCursor();
	QTextList *textList = cursor.currentList();
	if (!textList) {return;}

	QTextListFormat format = textList->format();
	if (format.indent() == 0) {return;}
	format.setIndent(format.indent() - 1);
	textList->setFormat(format);
}

void TextEditWidget::sl_CreateTableAction_Triggered() {
	QTextTable* table = textField->textCursor().currentTable();
	if (table) {
		WARNING("Wrong button state");
		return;
	}

	table = textField->textCursor().insertTable(2, 2);
	QTextTableFormat format = table->format();
	format.setCellSpacing(0);
	table->setFormat(format);
	QTextTableCell cell = table->cellAt(0, 0);
	textField->setTextCursor(cell.firstCursorPosition());
}

void TextEditWidget::sl_InsertRowAction_Triggered() {
	QTextTable* table = textField->textCursor().currentTable();
	if (!table) {
		WARNING("Wrong button state");
		return;
	}

	QTextTableCell currentCell = table->cellAt(textField->textCursor());
	table->insertRows(currentCell.row() + 1, 1);
	QTextTableCell cell = table->cellAt(currentCell.row() + 1, 0);
	textField->setTextCursor(cell.firstCursorPosition());
}

void TextEditWidget::sl_InsertColumnAction_Triggered() {
	QTextTable* table = textField->textCursor().currentTable();
	if (!table) {
		WARNING("Wrong button state");
		return;
	}

	QTextTableCell currentCell = table->cellAt(textField->textCursor());
	table->insertColumns(currentCell.column() + 1, 1);
	QTextTableCell cell = table->cellAt(0, currentCell.column() + 1);
	textField->setTextCursor(cell.firstCursorPosition());
}

void TextEditWidget::sl_RemoveRowAction_Triggered() {
	QTextTable* table = textField->textCursor().currentTable();
	if (!table) {
		WARNING("Wrong button state");
		return;
	}

	int rowNumber = table->cellAt(textField->textCursor()).row();
	table->removeRows(rowNumber, 1);
}

void TextEditWidget::sl_RemoveColumnAction_Triggered() {
	QTextTable* table = textField->textCursor().currentTable();
	if (!table) {
		WARNING("Wrong button state");
		return;
	}

	int columnNumber = table->cellAt(textField->textCursor()).column();
	table->removeColumns(columnNumber, 1);
}

void TextEditWidget::sl_MergeCellsAction_Triggered() {
	QTextTable* table = textField->textCursor().currentTable();
	if (!table) {
		WARNING("Wrong button state");
		return;
	}

	table->mergeCells(textField->textCursor());
}

void TextEditWidget::sl_fontComboBoxCurrentFontChanged (const QFont& font) {
	textField->SetFontFamily(font.family());
}

void TextEditWidget::sl_fontSizeComboBoxCurrentIndexChanged (int) {
	int size = fontSizeComboBox->itemData(fontSizeComboBox->currentIndex()).toInt();
	textField->SetFontSize(size);
}

void TextEditWidget::sl_TextEdit_CursorPositionChanged() {
	ACTBold->setChecked(textField->fontWeight() == QFont::Bold);
	ACTItalic->setChecked(textField->fontItalic());
	ACTUnderline->setChecked(textField->fontUnderline());
	strikeOutAction->setChecked(textField->StrikedOut());
	ACTAlignLeft->setChecked(textField->GetAlignment() == Qt::AlignLeft);
	ACTAlignCenter->setChecked(textField->GetAlignment() == Qt::AlignHCenter);
	ACTAlignRight->setChecked(textField->GetAlignment() == Qt::AlignRight);
	alignJustifyAction->setChecked(textField->GetAlignment() == Qt::AlignJustify);


	listButton->blockSignals(true);
		QTextCursor cursor = textField->textCursor();
		QTextList *textList = cursor.currentList();
		listButton->setChecked(textList != 0);
		increaseListIndentAction->setVisible(textList != 0);
		decreaseListIndentAction->setVisible(textList != 0);
	listButton->blockSignals(false);


	fontComboBox->blockSignals(true);
		QFont temp = textField->currentFont();
		fontComboBox->setCurrentFont(temp);
	fontComboBox->blockSignals(false);

	fontSizeComboBox->blockSignals(true);
		int size = this->textField->currentFont().pointSize();
		int index = fontSizeComboBox->findData(size);
		if (index != -1) {
			fontSizeComboBox->setCurrentIndex(index);
		}
	fontSizeComboBox->blockSignals(false);

	QTextTable* table = textField->textCursor().currentTable();
	bool tablePresent = (table != 0);
	insertRowAction->setVisible(tablePresent);
	insertColumnAction->setVisible(tablePresent);
	removeRowAction->setVisible(tablePresent);
	removeColumnAction->setVisible(tablePresent);
	mergeCellsAction->setVisible(tablePresent);
}

void TextEditWidget::sl_TextEdit_SelectionChanged() {

}

/* virtual */
bool TextEditWidget::eventFilter (QObject* watched, QEvent* event) {
	if (watched != textField && watched != searchEdit) {
		return QObject::eventFilter(watched, event);
	}

	if (event->type() == QEvent::KeyPress) {
		QKeyEvent* e = dynamic_cast<QKeyEvent*>(event);
		if (!e) {
			WARNING("Casting error");
			return false;
		}
		if (watched == textField) { // intercept Ctrl+F event for search
			if (e->key() == Qt::Key_F && e->modifiers() == Qt::ControlModifier) {
				// open search widget
				searchFrame->setVisible(true);
				searchEdit->setFocus();
				searchEdit->selectAll();

				return true;
			} else if (e->key() == Qt::Key_F4) {
				// continue search
				continueSearch();

				return true;
			}
		} else if (watched == searchEdit) {
			if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
				// start search
				if (!searchEdit->text().isEmpty()) {
					searchFrame->setVisible(false);
					textField->setFocus();
					continueSearch();
				}

				return true;
			} else if ((e->key() == Qt::Key_Escape) ||
					   (e->key() == Qt::Key_F && e->modifiers() == Qt::ControlModifier)) {
				searchFrame->setVisible(false);
				textField->setFocus();

				return true;
			}
		}
	}

	return QObject::eventFilter(watched, event);
}

void TextEditWidget::continueSearch() {
	QString searchText = searchEdit->text().trimmed();
	if (searchText.isEmpty()) {return;}
	if (!searchRegex->isChecked()) {
		searchText = QRegExp::escape(searchText);
	}
	if (searchWholeWord->isChecked()) {
		searchText.prepend("\\b");
		searchText.append("\\b");
	}

	int searchPos = textField->textCursor().selectionEnd();

	Qt::CaseSensitivity cs = searchMatchCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
	QRegExp regexp (searchText, cs);
	if (!regexp.isValid()) {
		searchFrame->setVisible(true);
		QToolTip::showText(searchEdit->mapToGlobal(QPoint(0, 0)), "Regexp is invalid", searchEdit);
		searchEdit->setFocus();
		searchEdit->selectAll();
		return;
	}


	int newPos = regexp.indexIn(textField->toPlainText(), searchPos);
	int newLength = regexp.matchedLength();

	if (newPos == -1 || newLength == -1) {return;}

	QTextCursor cursor(textField->document());
	cursor.setPosition(newPos);
	cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, newLength);
	textField->setTextCursor(cursor);
}

void TextEditWidget::ScrollTo(int position) {
	QTextCursor cursor = textField->textCursor();
	cursor.setPosition(position);
	textField->setTextCursor(cursor);
	textField->ensureCursorVisible();
}

void TextEditWidget::ShowFragment(const NoteFragment& fragment) {
	QTextCursor cursor = textField->textCursor();
	cursor.setPosition(fragment.Start);
	cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, fragment.Length);
	textField->setFocus();
	textField->setTextCursor(cursor);
	textField->ensureCursorVisible();
}

int TextEditWidget::CurrentPosition() const {
	return textField->textCursor().position();
}

void TextEditWidget::SetDocument(TextDocument* document) {
	textField->SetDocument(document);
}

QList<QAction*> TextEditWidget::EditActionsList() const {
	return publicActionsList;
}

void TextEditWidget::sl_TextEdit_UndoAvaliable(bool avaliable) {
	undoAction->setEnabled(avaliable);
}

void TextEditWidget::sl_TextEdit_RedoAvaliable(bool avaliable) {
	redoAction->setEnabled(avaliable);
}
