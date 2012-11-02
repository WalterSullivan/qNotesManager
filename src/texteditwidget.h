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

#ifndef TEXTEDITWIDGET_H
#define TEXTEDITWIDGET_H

#include <QWidget>
#include <QFrame>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include <QComboBox>
#include <QFontComboBox>


/*
  This is class inherited from QWidget, it groups TextEdit class object and a toolbar with text formatting
actions
*/

/*
  Add underline style:
	UnderlineStyle QTextCharFormat::underlineStyle () const
*/

#include "notefragment.h"

namespace qNotesManager {
	class ColorPickerButton;
	class TextEdit;
	class TextDocument;

	class TextEditWidget : public QWidget {
	Q_OBJECT
	private:
		// Toolbar
		QToolBar*			TBRMainBar;

		// Toolbar buttons
		QAction*			ACTClearFormatting;
		QAction*			copyFormatAction;
		QAction*			ACTBold;
		QAction*			ACTItalic;
		QAction*			ACTUnderline;
		QAction*			strikeOutAction;
		QAction*			ACTAlignLeft;
		QAction*			ACTAlignCenter;
		QAction*			ACTAlignRight;
		QAction*			alignJustifyAction;

		ColorPickerButton*	foregroundTextColorButton;
		ColorPickerButton*	backgroundTextColorButton;
		ColorPickerButton*	textUnderlineColorButton;

		QFontComboBox*		fontComboBox;
		QComboBox*			fontSizeComboBox;

		QFrame*				searchFrame;
		QLineEdit*			searchEdit;
		QCheckBox*			searchRegex;
		QCheckBox*			searchMatchCase;
		QCheckBox*			searchWholeWord;

		QToolButton*		listButton;
		QAction*			increaseListIndentAction;
		QAction*			decreaseListIndentAction;

		QAction*			createTableAction;
		QAction*			insertRowAction;
		QAction*			insertColumnAction;
		QAction*			removeRowAction;
		QAction*			removeColumnAction;
		QAction*			mergeCellsAction;

		QAction*			undoAction;
		QAction*			redoAction;

		// TextEdit itself
		TextEdit*			textField;

		QList<QAction*> publicActionsList;

		void CreateControls();
		void continueSearch();

		bool readOnly;


	public:
		explicit TextEditWidget (QWidget *parent = 0);

		QString toPlainText () const;
		QString toHtml () const;
		void setHtml (const QString& text);
		void setPlainText (const QString& text);

		QVariant GetResource(QString id);
		void SetResource (QString id, QVariant resource);
		/*virtual*/ bool eventFilter (QObject* watched, QEvent* event);

		void ScrollTo(int position);
		void ShowFragment(const NoteFragment& fragment);
		int CurrentPosition() const;
		void SetDocument(TextDocument*);
		QList<QAction*> EditActionsList() const;

		void SetReadOnly(bool);


	signals:
		void sg_TextChanged();

	private slots:
		void sl_ClearFormattingAction_Clicked();

		void sl_BoldAction_Triggered (bool toggle);
		void sl_ItalicAction_Triggered (bool toggle);
		void sl_UnderlineAction_Triggered (bool toggle);
		void sl_StrikeoutAction_Triggered (bool toggle);
		void sl_AlignLeftAction_Triggered (bool toggle);
		void sl_AlignCenterAction_Triggered (bool toggle);
		void sl_AlignRightAction_Triggered (bool toggle);
		void sl_AlignJustifyAction_Triggered(bool);
		void sl_ListButton_Toggled (bool toggle);
		void sl_ListButton_Triggered(QAction*);
		void sl_IncreaseListIndent_Triggered();
		void sl_DecreaseListIndent_Triggered();
		void sl_fontComboBoxCurrentFontChanged (const QFont & font);
		void sl_fontSizeComboBoxCurrentIndexChanged (int index);

		void sl_CreateTableAction_Triggered();
		void sl_InsertRowAction_Triggered();
		void sl_InsertColumnAction_Triggered();
		void sl_RemoveRowAction_Triggered();
		void sl_RemoveColumnAction_Triggered();
		void sl_MergeCellsAction_Triggered();

		void sl_ForegroundTextColorButton_Clicked();
		void sl_BackgroundTextColorButton_Clicked();
		void sl_TextUnderlineColorButton_Clicked();

		void sl_TextEdit_CursorPositionChanged();
		void sl_TextEdit_SelectionChanged();
		void sl_TextEdit_UndoAvaliable(bool);
		void sl_TextEdit_RedoAvaliable(bool);

	public slots:

	};
}

#endif // TEXTEDITWIDGET_H
 
