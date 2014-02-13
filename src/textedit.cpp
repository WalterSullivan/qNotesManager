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

#include "textedit.h"

#include "textdocument.h"
#include "hyperlinkeditwidget.h"
#include "global.h"
#include "edittablewidthconstraintswidget.h"

#include <QMimeData>
#include <QDebug>
#include <QTextBlock>
#include <QTextFragment>
#include <QKeyEvent>
#include <QMenu>
#include <QFileInfo>
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QImageReader>
#include <QToolTip>
#include <QInputDialog>
#include <QFileDialog>
#include <QTextTable>
#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>


using namespace qNotesManager;

TextEdit::TextEdit(QWidget *parent) :
	QTextEdit(parent),
	InsertHyperlinkAction(new QAction("Insert hyperlink", this)),
	InsertImageFromUrlAction(new QAction("Insert image from URL", this)),
	InsertImageFromFileAction(new QAction("Insert image from file", this)),
	InsertPlainTextAction(new QAction("Insert plain text", this)),
	InsertLineAction(new QAction("Insert line", this)),
	InsertDateTimeAction(new QAction("Insert date and time", this))
{
	followLinkAction = new QAction(tr("Follow Link"),this);
	QObject::connect(followLinkAction, SIGNAL(triggered()),
					 this, SLOT(sl_FollowLinkAction_Triggereed()));

	removeLinkAction = new QAction("Remove link", this);
	QObject::connect(removeLinkAction, SIGNAL(triggered()),
					 this, SLOT(sl_RemoveLinkAction_Triggered()));

	editLinkAction = new QAction("Edit link", this);
	QObject::connect(editLinkAction, SIGNAL(triggered()),
					 this, SLOT(sl_EditLinkActionTriggered()));

	tableAlignMenu = new QMenu("Table alignment", this);
	tableAlignLeft = new QAction("Left", this);
	QObject::connect(tableAlignLeft, SIGNAL(triggered()),
					 this, SLOT(sl_TableAlignAction_Triggered()));
	tableAlignRight = new QAction("Right", this);
	QObject::connect(tableAlignRight, SIGNAL(triggered()),
					 this, SLOT(sl_TableAlignAction_Triggered()));
	tableAlignCenter = new QAction("Center", this);
	QObject::connect(tableAlignCenter, SIGNAL(triggered()),
					 this, SLOT(sl_TableAlignAction_Triggered()));
	tableAlignMenu->addAction(tableAlignLeft);
	tableAlignMenu->addAction(tableAlignRight);
	tableAlignMenu->addAction(tableAlignCenter);


	addAction(InsertHyperlinkAction);
	QObject::connect(InsertHyperlinkAction, SIGNAL(triggered()),
					 this, SLOT(sl_InsertHyperlinkAction_Triggered()));

	addAction(InsertImageFromUrlAction);
	QObject::connect(InsertImageFromUrlAction, SIGNAL(triggered()),
					 this, SLOT(sl_InsertImageFromUrlAction_Triggered()));

	addAction(InsertImageFromFileAction);
	QObject::connect(InsertImageFromFileAction, SIGNAL(triggered()),
					 this, SLOT(sl_InsertImageFromFileAction_Triggered()));

	addAction(InsertPlainTextAction);
	InsertPlainTextAction->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_V));
	QObject::connect(InsertPlainTextAction, SIGNAL(triggered()),
					 this, SLOT(sl_InsertPlainTextAction_Triggered()));

	/*
	addAction(InsertLineAction);
	QObject::connect(InsertLineAction, SIGNAL(triggered()),
					 this, SLOT(sl_InsertLineAction_Triggered()));*/

	addAction(InsertDateTimeAction);
	QObject::connect(InsertDateTimeAction, SIGNAL(triggered()),
					 this, SLOT(sl_InsertDateTimeAction_Triggered()));

	setMouseTracking(true);

	QObject::connect(this, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
					 this, SLOT(sl_currentCharFormatChanged(QTextCharFormat)));

	linkEditDialog = new HyperlinkEditWidget(this);
	QObject::connect(&anchorTooltipTimer, SIGNAL(timeout()),
					 this, SLOT(sl_AnchorTooltipTimer_Timeout()));

	insertMenu = new QMenu("Insert...", this);
	insertMenu->addAction(InsertHyperlinkAction);
	insertMenu->addAction(InsertImageFromFileAction);
	insertMenu->addAction(InsertImageFromUrlAction);
	insertMenu->addAction(InsertPlainTextAction);
	//insertMenu->addAction(InsertLineAction);
	insertMenu->addAction(InsertDateTimeAction);

	imagePropertiesMenu = new QMenu("Image", this);
	saveImageAction = new QAction("Save image", this);
	QObject::connect(saveImageAction, SIGNAL(triggered()),
					 this, SLOT(sl_SaveImageAction_Triggered()));
	resizeImageAction = new QAction("Resize image", this);
	QObject::connect(resizeImageAction, SIGNAL(triggered()),
					 this, SLOT(sl_ResizeImageAction_Triggered()));
	resizeImageCanvasAction = new QAction("Resize image canvas", this);
	QObject::connect(resizeImageCanvasAction, SIGNAL(triggered()),
					 this, SLOT(sl_ResizeImageCanvasAction_Triggered()));

	imagePropertiesMenu->addAction(saveImageAction);
	imagePropertiesMenu->addAction(resizeImageAction);
	imagePropertiesMenu->addAction(resizeImageCanvasAction);

	editTableWidthConstraintsAction = new QAction(QIcon(":gui/resize"), "Edit table width...", this);
	QObject::connect(editTableWidthConstraintsAction, SIGNAL(triggered()),
					 this, SLOT(sl_EditTableWidthConstraintsAction_Triggered()));
}

void TextEdit::SetDocument(TextDocument* newDocument) {
	// QTextEdit class has no 'DocumentChanged' signal therefore this function is used to
	// manage signal-slot connections with current document
	TextDocument* d = 0;
	d = qobject_cast<TextDocument*>(document());
	if (d != 0) {
		QObject::disconnect(d, 0, this, 0);
	}
	d = 0;

	QObject::connect(newDocument, SIGNAL(sg_NeedRelayout()),
					 this, SLOT(sl_Document_NeedRelayout()));

	QTextEdit::setDocument(newDocument);
}

//virtual
void TextEdit::insertFromMimeData(const QMimeData* source) {
	/*QStringList list = source->formats();
	qDebug() << "\n\nMimeData dropped";
	qDebug() << "\tHas text: " << source->hasText();
	if (source->hasText()) {
		qDebug() << "\t\t" << source->text();
	}
	qDebug() << "\tHas html: " << source->hasHtml();
	if (source->hasHtml()) {
		qDebug() << "\t\t" << source->html();
	}
	qDebug() << "\tHas urls: " << source->hasUrls();
	qDebug() << "\tHas image: " << source->hasImage();
	qDebug() << "\n\tData:";

	foreach (QString s, list){
		qDebug() << "\t" << s;
		QByteArray array = source->data(s);
		qDebug() << "\t" << QString(array) << "\n";
	}*/


	if (source->hasImage()) { // paste image
		//QImage image = source->imageData().value<QImage>();
		//image.setText("FORMAT", "PNG"); // use PNG format to store unknown images
		//TextDocument* doc = dynamic_cast<TextDocument*>(document());
		//doc->InsertImage(image, textCursor());

	} else if (source->hasUrls()) {
		// paste urls, for example from file manager
		foreach (QUrl url, source->urls()) {
			insertImageFromFile(url.toLocalFile());
		}
	} else {
		QTextEdit::insertFromMimeData(source);
	}
}

void TextEdit::insertImageFromFile(QString fileName) {
	textCursor().insertImage(QUrl::fromLocalFile(fileName).toString());
}

void TextEdit::sl_InsertImageFromUrlAction_Triggered() {
	bool ok;
	QString text = QInputDialog::getText(this, "Insert image","Image URL:", QLineEdit::Normal,
										 "", &ok);
	if (ok && !text.isEmpty()) {
		QUrl url(text);
		if (!url.isValid()) {
			QMessageBox::information(this, "Insert image", "Specified URL is not valid");
			return;
		}
		textCursor().insertImage(url.toString());
	}
}

void TextEdit::sl_InsertImageFromFileAction_Triggered() {
	QString filter = "Images (";
	QList<QByteArray> formats = QImageReader::supportedImageFormats();
	foreach (QByteArray a, formats) {
		filter.append("*.").append(a).append(" ");
	}
	filter.replace(filter.length() - 1, 1, ")");

	QStringList list = QFileDialog::getOpenFileNames(this, "Insert image", "", filter, &filter);

	foreach (QString s, list) {
		insertImageFromFile(s);
	}
}

bool TextEdit::canInsertFromMimeData(const QMimeData* source) const {
	return source->hasImage() || source->hasUrls() || QTextEdit::canInsertFromMimeData(source);
}

//virtual
void TextEdit::keyPressEvent (QKeyEvent* e) {
	QTextEdit::keyPressEvent(e);

#ifdef DEBUG
	if (e->key() == Qt::Key_F5) {
		AnalyzeText();
	}

	if (e->key() == Qt::Key_H && e->modifiers() == Qt::ControlModifier) {
		qDebug() << document()->toHtml();
	}

	if (e->key() == Qt::Key_N && e->modifiers() == Qt::ControlModifier) {
		qDebug() << "\nSelection analysis:";
		QTextCursor cursor = this->textCursor();
		int start = cursor.selectionStart();
		int end = cursor.selectionEnd();
		int cur = start;
		QTextBlock block = document()->findBlock(cur);
		QList<QTextCursor> cursors;


		while (true) {
			if (!block.isValid()) {break;}
			if (block.blockNumber() == -1) {
				qDebug() << "Block blockNumber is -1. Pos: " << block.position();
				qDebug() << "Eixting";
				break;
			}
			if (block.position() > end) {break;}
			QTextBlock::iterator it;
			qDebug() << "Text block #" << block.blockNumber() << ". Text:\n" << block.text();
			for(it = block.begin(); !(it.atEnd()); ++it) {
				QTextFragment currentFragment = it.fragment();
				if (!currentFragment.isValid()) {continue;}
				int fs = currentFragment.position();
				int fe = currentFragment.position() + currentFragment.length();

				if (currentFragment.charFormat().isAnchor()) {continue;}
				if (end < fs || start > fe) {continue;} // skip not affected fragments

				qDebug() << "Fragment found: " << currentFragment.text() << ", fragment start: " << fs << ", fragment end: " << fe;

				QTextCursor temp(document());

				if (start < fs) {
					temp.setPosition(fs);
				} else {
					temp.setPosition(start);
				}

				if (end < fe) {
					temp.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, end - start);
				} else {
					temp.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, fe - start);
				}

				qDebug() << "Temp cursor. Pos: " << temp.selectionStart() << ", end: " << temp.selectionEnd() << ", length: " << temp.selectionEnd() - temp.selectionStart();
				cursors.append(temp);

			}

			block = block.next();
		}

		foreach (QTextCursor c, cursors) {
			QTextCharFormat frm;
			frm.setForeground(QBrush(QColor(255, 0, 0)));
			c.beginEditBlock();
			c.mergeCharFormat(frm);
			c.endEditBlock();
		}
	}

	if (e->key() == Qt::Key_Q && e->modifiers() == Qt::ControlModifier) {
		QClipboard *clipboard = QApplication::clipboard();
		const QMimeData* source = clipboard->mimeData();



		qDebug() << "\n\nMimeData dropped";
		qDebug() << "\tHas text: " << source->hasText();
		if (source->hasText()) {
			qDebug() << "\t\t" << source->text();
		}
		qDebug() << "\tHas html: " << source->hasHtml();
		if (source->hasHtml()) {
			qDebug() << "\t\t" << source->html();
		}
		qDebug() << "\tHas urls: " << source->hasUrls();
		qDebug() << "\tHas image: " << source->hasImage();
		qDebug() << "\n\tData:";

		QStringList list = source->formats();
		foreach (QString s, list){
			qDebug() << "\t" << s;
			qDebug() << "\t" << source->data(s) << "\n";
		}
	}
#endif


	QString href = anchorAt(mapFromGlobal(QCursor::pos()));
	if (e->modifiers() == Qt::ControlModifier) {
		if (!href.isEmpty() && viewport()->cursor().shape() != Qt::PointingHandCursor) {
			viewport()->setCursor(Qt::PointingHandCursor);
		}
	}
}

/* virtual */
void TextEdit::keyReleaseEvent (QKeyEvent* event) {
	QTextEdit::keyReleaseEvent(event);

	if (((event->modifiers() & Qt::ControlModifier) != Qt::ControlModifier) &&
				(viewport()->cursor().shape() != Qt::IBeamCursor)) {
			viewport()->setCursor(Qt::IBeamCursor);
	}
}

void TextEdit::AnalyzeText(){
	qDebug() << "\n------------------\nAnalyzing text\n";
	qDebug() << "HTML:";
	qDebug() << toHtml();
	QTextBlock bl = this->document()->begin();
	while(bl.isValid()) {
		QTextBlock::iterator it;
		qDebug() << "Text block #" << bl.blockNumber() << ". Text:\n" << bl.text() << "\n";
		for(it = bl.begin(); !(it.atEnd()); ++it) {
			QTextFragment currentFragment = it.fragment();
			if(currentFragment.isValid()) {
				qDebug() << "Text fragment #" << currentFragment.position() << ". Text:" << currentFragment.text() << "\n";
				if(currentFragment.charFormat().isImageFormat()) {
					QTextImageFormat imgFmt = currentFragment.charFormat().toImageFormat();

					QString image_name=imgFmt.name();
					qDebug() << "Image " << image_name << "\n";
				}
			}
		}
		bl = bl.next();
	}
}

//virtual
void TextEdit::contextMenuEvent (QContextMenuEvent* event) {
	QString anchor = anchorAt(event->pos());

	QMenu *menu = createStandardContextMenu();
	menu->addSeparator();

	if (!anchor.isEmpty()) {
		followLinkAction->setData(event->pos());
		removeLinkAction->setData(event->pos());
		editLinkAction->setData(event->pos());

		menu->addAction(this->followLinkAction);
		menu->addAction(this->removeLinkAction);
		menu->addAction(this->editLinkAction);
		menu->addSeparator();
	}

	menu->addMenu(insertMenu);

	if (textCursor().currentTable()) {
		menu->addSeparator();
		menu->addMenu(tableAlignMenu);
		menu->addSeparator();
		menu->addAction(editTableWidthConstraintsAction);
	}

	/* NOT IMPLEMENTED YET
	QTextFragment fragment = findFragmentAtPos(event->pos());
	if (fragment.isValid() && fragment.charFormat().isImageFormat()) {
		menu->addSeparator();
		menu->addMenu(imagePropertiesMenu);
		saveImageAction->setData(event->pos());
		resizeImageAction->setData(event->pos());
		resizeImageCanvasAction->setData(event->pos());
	}*/

	menu->exec(event->globalPos());
	menu->deleteLater();
}

//virtual
void TextEdit::mouseMoveEvent (QMouseEvent* event) {
	QTextEdit::mouseMoveEvent(event);

	QString href = anchorAt(event->pos());
	if (!href.isEmpty()) {
		anchorTooltipTimer.start(1000);

		if (event->modifiers() == Qt::ControlModifier &&
				viewport()->cursor().shape() != Qt::PointingHandCursor) {
			viewport()->setCursor(Qt::PointingHandCursor);
		}
	} else {
		if (viewport()->cursor().shape() != Qt::IBeamCursor) {

			viewport()->setCursor(Qt::IBeamCursor);
		}
		if (anchorTooltipTimer.isActive()) {anchorTooltipTimer.stop();}
		if (QToolTip::isVisible()) {QToolTip::hideText();}
	}
}

//virtual
void TextEdit::mousePressEvent (QMouseEvent* event) {
	QTextEdit::mousePressEvent(event);

	if (event->modifiers() == Qt::ControlModifier && event->button() == Qt::LeftButton) {

		QString href = anchorAt(event->pos());
		if (!href.isEmpty()) {
			QUrl url(href);
			QString scheme = url.scheme();

			if (scheme == "note") {
				// Not supported yet
			} else {
				QDesktopServices::openUrl(href);
			}
			emit this->sg_LinkClicked(url);
		}
	}
}

/* virtual */
void TextEdit::mouseReleaseEvent (QMouseEvent* event) {
	QTextEdit::mouseReleaseEvent(event);

	if (formatToCopy.isValid()) {
		QTextCharFormat format = formatToCopy.toCharFormat();
		applyCharFormatting(format, true, Set);
		formatToCopy = QTextFormat(QTextFormat::InvalidFormat);
		emit sg_CopyFormatCleared(false);
	}

}

void TextEdit::sl_currentCharFormatChanged (const QTextCharFormat&) {
	//qDebug() << "Format changed";
}

void TextEdit::sl_Document_contentsChange (int position, int charsRemoved, int charsAdded) {
	(void)position;
	(void)charsRemoved;
	(void)charsAdded;
}

void TextEdit::sl_Document_NeedRelayout() {
	setLineWrapColumnOrWidth (0);
}

void TextEdit::sl_FollowLinkAction_Triggereed() {
	QAction* act = qobject_cast<QAction*>(QObject::sender());
	QPoint pos = act->data().toPoint();

	QString anchor = anchorAt(pos);
	QUrl url(anchor);
	emit this->sg_LinkClicked(url);
}

void TextEdit::sl_RemoveLinkAction_Triggered() {
	QAction* act = qobject_cast<QAction*>(QObject::sender());
	QPoint pos = act->data().toPoint();

	QTextFragment currentFragment = findFragmentAtPos(pos);
	if (!currentFragment.isValid()) {
		qDebug() << "Found invalid fragment.";
		return;
	}

	QTextCharFormat format = currentFragment.charFormat();
	if (!format.isAnchor()) {
		qDebug() << "Found fragment isn't anchor";
		return;
	}

	QTextCursor cursor(document());
	format.clearProperty(QTextFormat::IsAnchor);
	format.clearProperty(QTextFormat::AnchorHref);
	format.clearProperty(QTextFormat::AnchorName);
	format.clearProperty(QTextFormat::ForegroundBrush);
	format.setFontUnderline(false);
	cursor.setPosition(currentFragment.position());
	cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, currentFragment.length());
	cursor.beginEditBlock();
	cursor.setCharFormat(format);
	cursor.endEditBlock();
	return;

}

void TextEdit::sl_EditLinkActionTriggered() {
	QAction* act = qobject_cast<QAction*>(QObject::sender());
	QPoint pos = act->data().toPoint();

	QTextFragment currentFragment = findFragmentAtPos(pos);
	if (!currentFragment.isValid()) {
		qDebug() << "Found invalid fragment.";
		return;
	}
	QTextCharFormat format = currentFragment.charFormat();
	if (!format.isAnchor()) {
		qDebug() << "Found fragment isn't anchor";
		return;
	}

	linkEditDialog->Set(currentFragment.text(),
						format.property(QTextFormat::AnchorHref).toString());

	if (linkEditDialog->exec() == QDialog::Rejected) {return;}

	format.setProperty(QTextFormat::AnchorHref, linkEditDialog->GetUrl());

	QTextCursor cursor(document());
	cursor.setPosition(currentFragment.position());
	cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, currentFragment.length());
	cursor.beginEditBlock();
	cursor.setCharFormat(format);
	cursor.insertText(linkEditDialog->GetName());
	cursor.endEditBlock();
}

void TextEdit::sl_InsertHyperlinkAction_Triggered() {
	linkEditDialog->Set("", "");
	if (linkEditDialog->exec() == QDialog::Rejected) {return;}

	QTextCursor cursor = textCursor();
	QString html = QString("<a href=\"%1\">%2</a>").arg(linkEditDialog->GetUrl()).arg(linkEditDialog->GetName());
	cursor.insertHtml(html);
}

void TextEdit::sl_TableAlignAction_Triggered() {
	QAction* const act = qobject_cast<QAction*>(QObject::sender());
	Qt::Alignment a;
	if (act == tableAlignLeft) {
		a = Qt::AlignLeft;
	} else if (act == tableAlignRight) {
		a = Qt::AlignRight;
	} else if (act == tableAlignCenter) {
		a = Qt::AlignHCenter;
	} else {
		WARNING("Unknown sender");
		return;
	}


	QTextTable* table = textCursor().currentTable();
	if (!table) {return;}

	QTextTableFormat format = table->format();
	format.setAlignment(a);
	table->setFormat(format);
}

QTextFragment TextEdit::findFragmentAtPos(QPoint pos) {
	QTextCursor cursor = cursorForPosition(pos);
	qDebug() << "Cursor data: start: " << cursor.position() << ", end: " << cursor.selectionEnd() - cursor.position();
	QTextBlock block = cursor.block();

	if (!block.isValid()) {return QTextFragment();}
	QTextBlock::iterator it;
	for(it = block.begin(); !(it.atEnd()); ++it) {
		QTextFragment currentFragment = it.fragment();
		if (!currentFragment.isValid()) {continue;}
		if (cursor.position() >= currentFragment.position() &&
			cursor.position() <= currentFragment.position() + currentFragment.length()) {
			return currentFragment;
		}
	}
	return QTextFragment();
}

void TextEdit::ClearFormatting() {
	QTextCharFormat f;
	applyCharFormatting(f, true, Set);
}

void TextEdit::SetSelectionForeColor(QColor color) {
	QTextCharFormat f;
	f.setForeground(QBrush(color));
	applyCharFormatting(f, true);
}

void TextEdit::SetSelectionBackColor(QColor color) {
	QTextCharFormat f;
	f.setBackground(QBrush(color));
	applyCharFormatting(f, true);
}

void TextEdit::SetSelectionBold(bool bold) {
	QTextCharFormat f;
	if (bold) {
		f.setFontWeight(QFont::Bold);
	} else {
		f.setFontWeight(QFont::Normal);
	}
	applyCharFormatting(f, false);
}

void TextEdit::SetSelectionItalic(bool italic) {
	QTextCharFormat f;
	f.setFontItalic(italic);
	applyCharFormatting(f, false);
}

void TextEdit::SetSelectionUnderline(bool underline) {
	QTextCharFormat f;

	f.setFontUnderline(underline);

	if (underline && !textCursor().charFormat().underlineColor().isValid()) {
		f.setUnderlineColor(Qt::black);
	} else if (!underline) {
		f.setUnderlineColor(QColor());
	}
	applyCharFormatting(f, true);
}

void TextEdit::SetSelectionUnderlineColor(QColor color) {
	QTextCharFormat f;

	if (!textCursor().charFormat().fontUnderline()) {
		f.setFontUnderline(true);
	}

	f.setUnderlineColor(color);
	applyCharFormatting(f, true);
}

void TextEdit::SetSelectionUnderlineStyle(QTextCharFormat::UnderlineStyle style) {
	QTextCharFormat f;
	f.setUnderlineStyle(style);
	applyCharFormatting(f, true);
}

void TextEdit::SetSelectionStrikeOut(bool strikeout) {
	QTextCharFormat f;
	f.setFontStrikeOut(strikeout);
	applyCharFormatting(f, false);
}

void TextEdit::SetAlignment(Qt::Alignment a) {
	setAlignment(a);
}

void TextEdit::SetFontFamily(QString family) {
	QTextCharFormat f;
	f.setFontFamily(family);
	applyCharFormatting(f, false);
}

void TextEdit::SetFontSize(qreal size) {
	QTextCharFormat f;
	f.setFontPointSize(size);
	applyCharFormatting(f, false);
}

QColor TextEdit::GetForeColor() const {
	QTextCharFormat format = textCursor().charFormat();
	return format.foreground().color();
}

QColor TextEdit::GetBackColor() const {
	QTextCharFormat format = textCursor().charFormat();
	QColor c = format.background().color();
	if (!format.background().isOpaque()) {c.setAlpha(0);}

	return c;
}

QColor TextEdit::GetUnderlineColor() const{
	QColor c;
	if (textCursor().charFormat().fontUnderline()) {
		QTextCharFormat format = textCursor().charFormat();
		c = format.underlineColor();
	}
	if (!c.isValid()) {c.setAlpha(0);}
	return c;
}

bool TextEdit::StrikedOut() const {
	return textCursor().charFormat().fontStrikeOut();
}

Qt::Alignment TextEdit::GetAlignment() const {
	return textCursor().blockFormat().alignment();
}

void TextEdit::applyCharFormatting(QTextCharFormat& format, bool skipLinks,
								   CharFormatApplyMode mode) {
	qDebug() << "applyCharFormatting";
	QTextCursor cursor = this->textCursor();
	int start = cursor.selectionStart();
	int end = cursor.selectionEnd();
	int cStart = 0;
	int cEnd = 0;
	qDebug() << "Selection: start: " << start << ", end: " << end;
	QTextBlock block = document()->findBlock(start);
	QList<QTextCursor> cursors;

	while (true) {
		if (!block.isValid()) {break;}
		if (block.blockNumber() == -1) {
			break;
		}
		if (block.position() > end) {break;}
		QTextBlock::iterator it;
		qDebug() << "Text block #" << block.blockNumber() << ". Text:\n" << block.text();
		for(it = block.begin(); !(it.atEnd()); ++it) {
			QTextFragment currentFragment = it.fragment();
			if (!currentFragment.isValid()) {continue;}
			int fs = currentFragment.position();
			int fe = currentFragment.position() + currentFragment.length();

			if (currentFragment.charFormat().isAnchor() && skipLinks) {continue;}
			if (end < fs || start > fe) {continue;} // skip not affected fragments


			QTextCursor temp(document());

			if (start < fs) {
				cStart = fs;
			} else {
				cStart = start;
			}

			if (end < fe) {
				cEnd = end;
			} else {
				cEnd = fe;
			}

			temp.setPosition(cStart);
			temp.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, cEnd - cStart);
			qDebug() << "Temp cursor. Pos: " << temp.selectionStart() << ", end: " << temp.selectionEnd() << ", length: " << temp.selectionEnd() - temp.selectionStart();

			cursors.append(temp);
		}

		block = block.next();
	}

	foreach (QTextCursor c, cursors) {
		c.beginEditBlock();
		if (mode == Merge) {
			c.mergeCharFormat(format);
		} else {
			c.setCharFormat(format);
		}
		c.endEditBlock();
	}
}

void TextEdit::sl_AnchorTooltipTimer_Timeout() {
	QString href = anchorAt(mapFromGlobal(QCursor::pos()));
	if (!href.isEmpty()) {
		QToolTip::showText(QCursor::pos(), href + "\n" + "Ctrl+Click to go", this);
	}
}

void TextEdit::sl_CopyCurrentFormat(bool copy) {
	if (copy) {
		formatToCopy = textCursor().charFormat();
	} else {
		formatToCopy = QTextFormat(QTextFormat::InvalidFormat);
	}
}

void TextEdit::sl_SaveImageAction_Triggered() {

}

void TextEdit::sl_ResizeImageAction_Triggered() {

}

void TextEdit::sl_ResizeImageCanvasAction_Triggered() {
	QAction* act = qobject_cast<QAction*>(QObject::sender());
	QPoint pos = act->data().toPoint();

	QTextFragment fragment = findFragmentAtPos(pos);
	if (fragment.isValid() && fragment.charFormat().isImageFormat()) {

	}
}

void TextEdit::sl_InsertPlainTextAction_Triggered() {
	QClipboard* clipboard = QApplication::clipboard();
	textCursor().insertText(clipboard->text());
}

void TextEdit::sl_InsertLineAction_Triggered() {
	textCursor().insertHtml("<hr>");
}

void TextEdit::sl_InsertDateTimeAction_Triggered() {
	QString text = QString("%1%2").arg(
			QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate)
			).arg("<br>");
	textCursor().insertHtml(text);
}

void TextEdit::sl_EditTableWidthConstraintsAction_Triggered() {
	QTextTable* table = textCursor().currentTable();
	if (!table) {
		WARNING("No table");
		return;
	}

	QTextTableFormat format = table->format();

	EditTableWidthConstraintsWidget* widget = new EditTableWidthConstraintsWidget(format, table->columns(), 0);
	widget->exec();
	if (widget->result() == QDialog::Accepted) {
		format.setWidth(widget->TableWidthConstraint);
		format.setColumnWidthConstraints(widget->ColumnWidthConstraints);
		table->setFormat(format);
	}

	delete widget;
}
