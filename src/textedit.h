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

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>
#include <QAction>
#include <QHash>
#include <QUrl>
#include <QTextFragment>
#include <QTimer>

/*
  This is class inherited from QTextEdit, it handles text appearing and all the stuff about it
*/

namespace qNotesManager {
	class HyperlinkEditWidget;
	class TextDocument;

	class TextEdit : public QTextEdit {
	Q_OBJECT
	public:
		explicit TextEdit(QWidget *parent = 0);

		void SetDocument(TextDocument*);

		void ClearFormatting();
		void SetSelectionForeColor(QColor);
		void SetSelectionBackColor(QColor);
		void SetSelectionBold(bool);
		void SetSelectionItalic(bool);
		void SetSelectionUnderline(bool);
		void SetSelectionUnderlineColor(QColor);
		void SetSelectionUnderlineStyle(QTextCharFormat::UnderlineStyle);
		void SetSelectionStrikeOut(bool);
		void SetAlignment(Qt::Alignment);
		void SetFontFamily(QString);
		void SetFontSize(qreal);

		QColor GetForeColor() const;
		QColor GetBackColor() const;
		QColor GetUnderlineColor() const;
		bool StrikedOut() const;
		Qt::Alignment GetAlignment() const;


		QAction* const InsertHyperlinkAction;
		QAction* const InsertImageFromUrlAction;
		QAction* const InsertImageFromFileAction;
		QAction* const InsertPlainTextAction;
		QAction* const InsertLineAction;
		QAction* const InsertDateTimeAction;

	protected:
		virtual void insertFromMimeData (const QMimeData * source);
		virtual bool canInsertFromMimeData(const QMimeData *source) const;

		virtual void contextMenuEvent (QContextMenuEvent * event);

		virtual void mouseMoveEvent (QMouseEvent* event);
		virtual void mousePressEvent (QMouseEvent* e);
		virtual void mouseReleaseEvent (QMouseEvent* e);
		virtual void keyPressEvent (QKeyEvent * e);
		virtual void keyReleaseEvent (QKeyEvent* event);

	private:

		enum CharFormatApplyMode {
			Merge,
			Set
		};

		void AnalyzeText();

		QAction* followLinkAction;
		QAction* removeLinkAction;
		QAction* editLinkAction;
		QMenu*   tableAlignMenu;
		QAction* tableAlignLeft;
		QAction* tableAlignRight;
		QAction* tableAlignCenter;

		QMenu* insertMenu;

		QMenu* imagePropertiesMenu;
		QAction* saveImageAction;
		QAction* resizeImageAction;
		QAction* resizeImageCanvasAction;

		QTextFormat formatToCopy;

		QAction* editTableWidthConstraintsAction;

		HyperlinkEditWidget* linkEditDialog;
		QTimer	anchorTooltipTimer;

		QTextFragment findFragmentAtPos(QPoint pos);
		void applyCharFormatting(QTextCharFormat& format, bool skipLinks, CharFormatApplyMode = Merge);
		void insertImageFromFile(QString fileName);

		void setDocument(QTextDocument*) {} // hide inherited function


	signals:
		void sg_LinkClicked(QUrl url);
		void sg_CopyFormatCleared(bool);

	private slots:
		void sl_currentCharFormatChanged (const QTextCharFormat& f);
		void sl_Document_contentsChange (int position, int charsRemoved, int charsAdded);
		void sl_Document_NeedRelayout();

		void sl_FollowLinkAction_Triggereed();
		void sl_RemoveLinkAction_Triggered();
		void sl_EditLinkActionTriggered();
		void sl_InsertHyperlinkAction_Triggered();
		void sl_InsertImageFromUrlAction_Triggered();
		void sl_InsertImageFromFileAction_Triggered();
		void sl_TableAlignAction_Triggered();
		void sl_InsertPlainTextAction_Triggered();
		void sl_InsertLineAction_Triggered();
		void sl_InsertDateTimeAction_Triggered();

		void sl_SaveImageAction_Triggered();
		void sl_ResizeImageAction_Triggered();
		void sl_ResizeImageCanvasAction_Triggered();

		void sl_AnchorTooltipTimer_Timeout();

		void sl_EditTableWidthConstraintsAction_Triggered();

	public slots:
		void sl_CopyCurrentFormat(bool);

	};
}

#endif // TEXTEDIT_H
