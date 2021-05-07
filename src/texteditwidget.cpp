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
#include "searchpanelwidget.h"
#include "tablepropertieswidget.h"

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

	textField->installEventFilter(this);

	QObject::connect(textField, SIGNAL(sg_LinkClicked(QUrl)),
					 this, SIGNAL(sg_LinkClicked(QUrl)));
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
	fontComboBox->setFocusPolicy(Qt::ClickFocus);
	TBRMainBar->addWidget(fontComboBox);

	fontSizeComboBox = new QComboBox();
	fontSizeComboBox->setFocusPolicy(Qt::ClickFocus);
	QFontDatabase db;
	foreach(int size, db.standardSizes()) {
		fontSizeComboBox->addItem(QString::number(size), double(size));
	}
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

	createTableAction = new QAction(QIcon(":/gui/table"), "Create table", this);
	QObject::connect(createTableAction, SIGNAL(triggered()),
					 this, SLOT(sl_CreateTableAction_Triggered()));

	insertRowAction = new QAction(QIcon(":/gui/table-insert-row"), "Insert row", this);
	QObject::connect(insertRowAction, SIGNAL(triggered()),
					 this, SLOT(sl_InsertRowAction_Triggered()));

	insertColumnAction = new QAction(QIcon(":/gui/table-insert-column"), "Insert column", this);
	QObject::connect(insertColumnAction, SIGNAL(triggered()),
					 this, SLOT(sl_InsertColumnAction_Triggered()));

	removeRowAction = new QAction(QIcon(":/gui/table-delete-row"), "Remove row", this);
	QObject::connect(removeRowAction, SIGNAL(triggered()),
					 this, SLOT(sl_RemoveRowAction_Triggered()));

	removeColumnAction = new QAction(QIcon(":/gui/table-delete-column"), "Remove column", this);
	QObject::connect(removeColumnAction, SIGNAL(triggered()),
					 this, SLOT(sl_RemoveColumnAction_Triggered()));

	mergeCellsAction = new QAction(QIcon(":/gui/table-join"), "Merge cells", this);
	QObject::connect(mergeCellsAction, SIGNAL(triggered()),
					 this, SLOT(sl_MergeCellsAction_Triggered()));

	tablePropertiesAction = new QAction(QIcon(":/gui/table-edit"), "Table properties", this);
	QObject::connect(tablePropertiesAction, SIGNAL(triggered()), this, SLOT(sl_TablePropertiesAction_Triggered()));

	createTableButton = new QToolButton();
	createTableButton->setFocusPolicy(Qt::NoFocus);
	createTableButton->setPopupMode(QToolButton::MenuButtonPopup);
	createTableButton->setCheckable(false);
	createTableButton->setDefaultAction(createTableAction);

	createTableButtonMenu = new QMenu(this);
	createTableButtonMenu->addAction(insertRowAction);
	createTableButtonMenu->addAction(insertColumnAction);
	createTableButtonMenu->addAction(removeRowAction);
	createTableButtonMenu->addAction(removeColumnAction);
	createTableButtonMenu->addAction(mergeCellsAction);
	createTableButtonMenu->addAction(tablePropertiesAction);
	createTableButton->setMenu(createTableButtonMenu);

	TBRMainBar->addWidget(createTableButton);
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

	TBRMainBar->addSeparator();

	showSpaceCharacters = TBRMainBar->addAction(QIcon(":/gui/edit-pilcrow"), "");
#if QT_VERSION >= 0x050000
	showSpaceCharacters->setText("Show special characters");
#else
	showSpaceCharacters->setText("Show space characters");
#endif
	showSpaceCharacters ->setCheckable(true);
	QObject::connect(showSpaceCharacters, SIGNAL(triggered(bool)), this, SLOT(sl_ShowSpaceCharacters_Triggered(bool)));

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
	/* Not implemented yet
	publicActionsList.append(textField->InsertLineAction);
	*/
	publicActionsList.append(textField->InsertDateTimeAction);

	// Create search widget
	searchPanel = new SearchPanelWidget(textField);

	// Create layout
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(TBRMainBar);
	layout->addWidget(textField);
	layout->addWidget(searchPanel);
	searchPanel->setVisible(false);
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

void TextEditWidget::SetReadOnly(bool ro) {
	if (readOnly == ro) {return;}

	readOnly = ro;

	TBRMainBar->setEnabled(!readOnly);
	textField->setReadOnly(readOnly);
}

void TextEditWidget::FocusTextEdit() {
	textField->setFocus();
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


void TextEditWidget::sl_ShowSpaceCharacters_Triggered(bool toggle) {
	QTextOption option = textField->document()->defaultTextOption();
	QTextOption::Flags flags = option.flags();

	if (toggle) {
		flags = flags | QTextOption::ShowTabsAndSpaces;
#if QT_VERSION >= 0x050000
		// Pilcrow character (paragraph separator) shows incorrectly in qt4
		flags = flags | QTextOption::ShowLineAndParagraphSeparators | QTextOption::AddSpaceForLineAndParagraphSeparators;
#endif
	} else {
		flags = flags & ~QTextOption::ShowTabsAndSpaces & ~QTextOption::ShowLineAndParagraphSeparators & ~QTextOption::AddSpaceForLineAndParagraphSeparators;
	}
	option.setFlags(flags);
	textField->document()->setDefaultTextOption(option);
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
	if (action == nullptr) {
		WARNING("Null pointer recieved");
		return;
	}
	QTextListFormat::Style style = (QTextListFormat::Style)action->data().toInt();

	QTextCursor cursor = this->textField->textCursor();
	QTextList *textList = cursor.currentList();
	if (textList == nullptr) {
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
	if (textList == nullptr) {return;}

	QTextListFormat format = textList->format();
	format.setIndent(format.indent() + 1);
	textList->setFormat(format);
}

void TextEditWidget::sl_DecreaseListIndent_Triggered() {
	QTextCursor cursor = this->textField->textCursor();
	QTextList *textList = cursor.currentList();
	if (textList == nullptr) {return;}

	QTextListFormat format = textList->format();
	if (format.indent() == 0) {return;}
	format.setIndent(format.indent() - 1);
	textList->setFormat(format);
}

void TextEditWidget::sl_CreateTableAction_Triggered() {
	QTextTable* table = textField->textCursor().currentTable();
	if (table) {
		return;
	}

	textField->textCursor().beginEditBlock();
	table = textField->textCursor().insertTable(2, 2);
	QTextTableFormat format = table->format();

	format.setCellSpacing(0);
	format.setCellPadding(3);
	format.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);

	QTextLength tableWidth(QTextLength::PercentageLength, 50);
	format.setWidth(tableWidth);

	table->setFormat(format);
	textField->textCursor().endEditBlock();

	QTextTableCell cell = table->cellAt(0, 0);
	textField->setTextCursor(cell.firstCursorPosition());
}

void TextEditWidget::sl_InsertRowAction_Triggered() {
	QTextTable* table = textField->textCursor().currentTable();
	if (table == nullptr) {
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
	if (table == nullptr) {
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
	if (table == nullptr) {
		WARNING("Wrong button state");
		return;
	}

	int rowNumber = table->cellAt(textField->textCursor()).row();
	table->removeRows(rowNumber, 1);
}

void TextEditWidget::sl_TablePropertiesAction_Triggered() {
	QTextTable* table = textField->textCursor().currentTable();

	if (table == nullptr) {return;}

	TablePropertiesWidget widget {table, this};
	widget.exec();
}

void TextEditWidget::sl_RemoveColumnAction_Triggered() {
	QTextTable* table = textField->textCursor().currentTable();
	if (table == nullptr) {
		WARNING("Wrong button state");
		return;
	}

	int columnNumber = table->cellAt(textField->textCursor()).column();
	table->removeColumns(columnNumber, 1);
}

void TextEditWidget::sl_MergeCellsAction_Triggered() {
	QTextTable* table = textField->textCursor().currentTable();
	if (table == nullptr) {
		WARNING("Wrong button state");
		return;
	}

	table->mergeCells(textField->textCursor());
}

void TextEditWidget::sl_fontComboBoxCurrentFontChanged (const QFont& font) {
	textField->SetFontFamily(font.family());
	textField->setFocus();
}

void TextEditWidget::sl_fontSizeComboBoxCurrentIndexChanged (int) {
	int size = fontSizeComboBox->itemData(fontSizeComboBox->currentIndex()).toInt();
	textField->SetFontSize(size);
	textField->setFocus();
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
		const QTextCursor cursor = textField->textCursor();
		const QTextList *textList = cursor.currentList();
		listButton->setChecked(textList != 0);
		increaseListIndentAction->setVisible(textList != 0);
		decreaseListIndentAction->setVisible(textList != 0);
	listButton->blockSignals(false);


	fontComboBox->blockSignals(true);
		const QFont temp = textField->currentFont();
		fontComboBox->setCurrentFont(temp);
	fontComboBox->blockSignals(false);

	fontSizeComboBox->blockSignals(true);
		while (true) {
			// Remove non-standard font size values
			const int index = fontSizeComboBox->findData(-1);
			if (index != -1) {fontSizeComboBox->removeItem(index);} else {break;}
		}

		const double size = this->textField->currentFont().pointSizeF();
		const int index = fontSizeComboBox->findData(size);
		if (index != -1) {
			fontSizeComboBox->setCurrentIndex(index);
		} else {
			// Font size is not standard, add new item to font size combobox
			int newIndex = 0;
			while (true) {
				bool ok = false;
				double tempSize = fontSizeComboBox->itemData(newIndex).toDouble(&ok);
				if (ok) {
					if (tempSize > size) {break;}
				}

				newIndex++;
				if (newIndex >= fontSizeComboBox->count()) {break;}
			}
			fontSizeComboBox->insertItem(newIndex, QString::number(size), -1);
			fontSizeComboBox->setCurrentIndex(newIndex);
		}
	fontSizeComboBox->blockSignals(false);

	const QTextTable* table = textField->textCursor().currentTable();
	const bool tablePresent = (table != nullptr);

	insertRowAction->setEnabled(tablePresent);
	insertColumnAction->setEnabled(tablePresent);
	removeRowAction->setEnabled(tablePresent);
	removeColumnAction->setEnabled(tablePresent);
	mergeCellsAction->setEnabled(tablePresent);
	tablePropertiesAction->setEnabled(tablePresent);
	if (tablePresent) {
		createTableAction->setText("Edit table...");
	} else {
		createTableAction->setText("Create table");
	}
}

void TextEditWidget::sl_TextEdit_SelectionChanged() {

}

/* virtual */
bool TextEditWidget::eventFilter (QObject* watched, QEvent* event) {
	if (watched != textField) {
		return QObject::eventFilter(watched, event);
	}

	if (event->type() != QEvent::KeyPress) {
		return QObject::eventFilter(watched, event);
	}

	QKeyEvent* e = dynamic_cast<QKeyEvent*>(event);
	if (e == nullptr) {
		WARNING("Casting error");
		return false;
	}

	QKeySequence keySequence(e->key() + e->modifiers());

	if (watched == textField) {
		if (keySequence == QKeySequence(QKeySequence::Find) ||
			keySequence == QKeySequence(QKeySequence::Replace)) {
			searchPanel->setVisible(true);

			if (searchPanel->SearchText().isEmpty() &&
				!textField->textCursor().selectedText().isEmpty()) {
				searchPanel->sl_SetSearchText(textField->textCursor().selectedText());
			}
			searchPanel->sl_SelectAllAndFocusSearchBox();

			if (keySequence == QKeySequence(QKeySequence::Replace) &&
				!searchPanel->SearchText().isEmpty()) {
				searchPanel->sl_SelectAllAndFocusReplaceBox();
			}

			return true;
		}
	}

	if (keySequence == QKeySequence(QKeySequence::FindNext)) {
		searchPanel->sl_FindNext();
		return true;

	} else if (keySequence == QKeySequence(QKeySequence::FindPrevious)) {
		searchPanel->sl_FindPrevious();
		return true;
	}

	return QObject::eventFilter(watched, event);
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

	undoAction->setEnabled(document->isUndoAvailable());
	redoAction->setEnabled(document->isRedoAvailable());
}

QList<QAction*> TextEditWidget::EditActionsList() const {
	return publicActionsList;
}

void TextEditWidget::UpdateActionsStatus(bool enabled) {
	foreach (QAction* action, publicActionsList) {
		if (action == undoAction || action == redoAction) {
			if (!enabled) {
				undoAction->setEnabled(false);
				redoAction->setEnabled(false);
			} else {
				undoAction->setEnabled(textField->document()->isUndoAvailable());
				redoAction->setEnabled(textField->document()->isRedoAvailable());
			}
		} else {
			action->setEnabled(enabled);
		}
	}
}

void TextEditWidget::sl_TextEdit_UndoAvaliable(bool avaliable) {
	undoAction->setEnabled(avaliable);
}

void TextEditWidget::sl_TextEdit_RedoAvaliable(bool avaliable) {
	redoAction->setEnabled(avaliable);
}
