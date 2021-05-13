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
#include "custommessagebox.h"
#include "sizeeditwidget.h"

#include <QMimeData>
#include <QDebug>
#include <QTextBlock>
#include <QTextFragment>
#include <QTextDocumentFragment>
#include <QKeyEvent>
#include <QMenu>
#include <QFileInfo>
#include <QClipboard>
#include <QApplication>
#include <QImageReader>
#include <QToolTip>
#include <QInputDialog>
#include <QFileDialog>
#include <QTextTable>
#include <QClipboard>
#include <QDateTime>
#include "cachedimagefile.h"
#include <QBuffer>


using namespace qNotesManager;

TextEdit::TextEdit(QWidget *parent) :
	QTextEdit(parent),
	InsertHyperlinkAction(new QAction(QIcon(":/gui/globe-network"), "Insert hyperlink", this)),
	InsertImageFromUrlAction(new QAction(QIcon(":/gui/image"), "Insert image from URL", this)),
	InsertImageFromFileAction(new QAction(QIcon(":/gui/image"), "Insert image from file", this)),
	InsertPlainTextAction(new QAction(QIcon(":/gui/edit"), "Insert plain text", this)),
	InsertLineAction(new QAction("Insert line", this)),
	InsertDateTimeAction(new QAction(QIcon(":/gui/date"), "Insert date and time", this))
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
	linkEditDialog->resize(400, 50);
	anchorTooltipTimer.setSingleShot(true);
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
	saveImageAction = new QAction(QIcon(":/gui/disk-black"), "Save image", this);
	QObject::connect(saveImageAction, SIGNAL(triggered()),
					 this, SLOT(sl_SaveImageAction_Triggered()));
	resizeImageAction = new QAction(QIcon(":/gui/image-resize"), "Resize image", this);
	QObject::connect(resizeImageAction, SIGNAL(triggered()),
					 this, SLOT(sl_ResizeImageAction_Triggered()));


	imagePropertiesMenu->addAction(saveImageAction);
	imagePropertiesMenu->addAction(resizeImageAction);
}

void TextEdit::SetDocument(TextDocument* newDocument) {
	// QTextEdit class has no 'DocumentChanged' signal therefore this function is used to
	// manage signal-slot connections with current document
	TextDocument* d = nullptr;
	d = qobject_cast<TextDocument*>(document());
	if (d != nullptr) {
		QObject::disconnect(d, 0, this, 0);
	}
	d = nullptr;

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
		// Try to find dropped image's hyperlink
		bool nameFound = false;
		QUrl imageFileUrl;

		if (source->hasHtml() && !nameFound) {
			QRegExp r("(<img)([^>]*)(src)([^\"]*)(\")([^\"]*)(\")");
			QString possibleFileName;
			if (r.indexIn(source->html(), 0) != -1) {
				possibleFileName = r.cap(6);

				QUrl url(possibleFileName);
				if (url.isValid()) {
					imageFileUrl = url;
					nameFound = true;
				}
			}
		}

		if (source->hasText() && !nameFound) {
			QUrl url = source->text();
			if (url.isValid()) {
				imageFileUrl = url;
				nameFound = true;
			}
		}

		if (source->hasUrls() && !nameFound) {
			QUrl url = source->urls().at(0);
			if (url.isValid()) {
				imageFileUrl = url;
				nameFound = true;
			}
		}

		qDebug() << "Final url of dropped image: " << imageFileUrl.toString();

		if (nameFound && imageFileUrl.isValid() && !imageFileUrl.isEmpty()) {
			textCursor().insertImage(imageFileUrl.toString());
			return;
		} else {
			// Unknown image
			QImage image = source->imageData().value<QImage>();
			if (image.isNull()) {
				return;
			}
			QByteArray imageData;
			QBuffer buffer(&imageData);
			buffer.open(QIODevice::WriteOnly);
			image.save(&buffer, "png");
			buffer.close();
			if (imageData.isEmpty()) {
				return;
			}

			CachedImageFile* cachedImage = new CachedImageFile(imageData, "unnamed", "png");
			QString name = cachedImage->GetMD5();
			TextDocument* doc = dynamic_cast<TextDocument*>(document());
			doc->AddResourceImage(cachedImage);
			textCursor().insertImage(name);
		}

	} else if (source->hasUrls()) {
		// paste urls, for example from file manager
		foreach (QUrl url, source->urls()) {
			if (!url.isValid()) {
				continue;
			}
			QString filePath = url.toString();
			QFileInfo info(filePath);
			QString fileType = info.suffix().toLower();

			if (fileType.isEmpty()) {continue;}

			if (QImageReader::supportedImageFormats().contains(fileType.toUtf8())) {
				// Dropped file is image file
				textCursor().insertImage(filePath);
			} else {
				// Check if dropped file is a text file
				if (url.scheme() == "file") {
					filePath = url.toLocalFile();
				}
				if (fileType == "txt") {
					QFile f(filePath);
					if (f.open(QIODevice::Text | QIODevice::ReadOnly)) {
						QByteArray fileData = f.readAll();
						f.close();
						QString fileText = QString(fileData);
						textCursor().insertText(fileText);
					}
				}
			}
		}
	} else {
		QTextEdit::insertFromMimeData(source);
	}
}

bool TextEdit::canInsertFromMimeData(const QMimeData* source) const {
	if (source->hasImage()) {
		return true;
	} else if (source->hasUrls()) {
		foreach (const QUrl& url, source->urls()) {
			if (!url.isValid()) {
				return false;
			}

			QString filePath = url.toString();
			QFileInfo info(filePath);
			QString fileType = info.suffix().toLower();
			if (fileType.isEmpty()) {return false;}

			if (
					!QImageReader::supportedImageFormats().contains(fileType.toUtf8()) &&
					!((url.scheme() == "file") && (fileType == "txt"))
				) {
					return false;
			}
		}
		return true;
	} else {
		return QTextEdit::canInsertFromMimeData(source);
	}
}

QMimeData* TextEdit::createMimeDataFromSelection() const {
	const TextDocument* d = qobject_cast<TextDocument*>(document());
	if (d == nullptr) {
		return QTextEdit::createMimeDataFromSelection();
	}

	QMimeData* data = new QMimeData();

	const QTextDocumentFragment selection = textCursor().selection();
	if (selection.isEmpty()) {
		return data;
	}

	QString html = selection.toHtml();
	QString text = selection.toPlainText();

	QRegExp regexp("(<img src=\")([^\"]*)(\")");
	int index = 0;

	while ((index = regexp.indexIn(html, index)) != -1) {
		QString url = regexp.cap(2);
		int pos = regexp.pos(2);
		int length = url.length();

		const CachedImageFile* image = d->GetResourceImage(url);
		if (image != nullptr) {
			const QString fileName = image->SaveToTempFolder();
			if (!fileName.isEmpty()) {
				const QUrl newUrl = QUrl::fromLocalFile(fileName);
				html.replace(pos, length, newUrl.toString());
			}
		}

		index += regexp.matchedLength();
	}

	data->setHtml(html);
	data->setText(text);

	return data;
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
			CustomMessageBox msg(this, "Specified URL is not valid", "Insert image", QMessageBox::Information);
			msg.show();
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

/* virtual */
void TextEdit::focusOutEvent(QFocusEvent* event) {
	if (anchorTooltipTimer.isActive()) {anchorTooltipTimer.stop();}

	QTextEdit::focusOutEvent(event);
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


	QTextFragment fragment = findFragmentAtPos(event->pos());
	if (fragment.isValid() && fragment.charFormat().isImageFormat()) {
		menu->addSeparator();
		menu->addMenu(imagePropertiesMenu);
		saveImageAction->setData(event->pos());
		resizeImageAction->setData(event->pos());
	}

	menu->exec(event->globalPos());
	menu->deleteLater();
}

//virtual
void TextEdit::mouseMoveEvent (QMouseEvent* event) {
	QTextEdit::mouseMoveEvent(event);

	QString href = anchorAt(event->pos());
	if (!href.isEmpty() && this->hasFocus()) {
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
			emit this->sg_LinkClicked(url);
		}
	}
}

/* virtual */
void TextEdit::mouseReleaseEvent (QMouseEvent* event) {
	QTextEdit::mouseReleaseEvent(event);

	if (charFormatToCopy.isValid()) {
		QTextCharFormat format = charFormatToCopy.toCharFormat();
		applyCharFormatting(format, Set);
		charFormatToCopy = QTextFormat(QTextFormat::InvalidFormat);
		emit sg_CopyFormatCleared(false);
	}
	if (blockFormatToCopy.isValid()) {
		QTextBlockFormat format = blockFormatToCopy.toBlockFormat();
		textCursor().setBlockFormat(format);
		blockFormatToCopy = QTextFormat(QTextFormat::InvalidFormat);
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
	format.setUnderlineColor(QColor());
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
	QString selectedText = textCursor().selection().toPlainText();

	QString urlText = "";
	const QMimeData* data = QApplication::clipboard()->mimeData();
	if (data->hasText()) {
		urlText = data->text();
	} else if (data->hasUrls()) {
		QUrl url = data->urls().at(0);
		urlText = url.toString();
	}

	if (selectedText.isEmpty()) {selectedText = urlText;}

	linkEditDialog->Set(selectedText, urlText);
	if (linkEditDialog->exec() == QDialog::Rejected) {return;}

	QTextCursor cursor = textCursor();
	QString html = QString("<a href=\"%1\">%2</a>").arg(linkEditDialog->GetUrl()).arg(linkEditDialog->GetName());
	cursor.insertHtml(html);
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
	QTextCursor cursor(textCursor());
	cursor.beginEditBlock();

	QTextCharFormat charFormat;
	QTextBlockFormat blockFormat;

	cursor.setCharFormat(charFormat);
	cursor.setBlockCharFormat(charFormat);
	cursor.setBlockFormat(blockFormat);
	cursor.endEditBlock();
}

void TextEdit::SetSelectionForeColor(QColor color) {
	QTextCharFormat f;
	f.setForeground(QBrush(color));
	applyCharFormatting(f);
}

void TextEdit::SetSelectionBackColor(QColor color) {
	QTextCharFormat f;
	f.setBackground(QBrush(color));
	applyCharFormatting(f);
}

void TextEdit::SetSelectionBold(bool bold) {
	QTextCharFormat f;
	if (bold) {
		f.setFontWeight(QFont::Bold);
	} else {
		f.setFontWeight(QFont::Normal);
	}
	applyCharFormatting(f);
}

void TextEdit::SetSelectionItalic(bool italic) {
	QTextCharFormat f;
	f.setFontItalic(italic);
	applyCharFormatting(f);
}

void TextEdit::SetSelectionUnderline(bool underline) {
	QTextCharFormat f;

	f.setFontUnderline(underline);

	if (underline && !textCursor().charFormat().underlineColor().isValid()) {
		f.setUnderlineColor(Qt::black);
	} else if (!underline) {
		f.setUnderlineColor(QColor());
	}
	applyCharFormatting(f);
}

void TextEdit::SetSelectionUnderlineColor(QColor color) {
	QTextCharFormat f;

	if (!textCursor().charFormat().fontUnderline()) {
		f.setFontUnderline(true);
	}

	f.setUnderlineColor(color);
	applyCharFormatting(f);
}

void TextEdit::SetSelectionUnderlineStyle(QTextCharFormat::UnderlineStyle style) {
	QTextCharFormat f;
	f.setUnderlineStyle(style);
	applyCharFormatting(f);
}

void TextEdit::SetSelectionStrikeOut(bool strikeout) {
	QTextCharFormat f;
	f.setFontStrikeOut(strikeout);
	applyCharFormatting(f);
}

void TextEdit::SetAlignment(Qt::Alignment a) {
	setAlignment(a);
}

void TextEdit::SetFontFamily(QString family) {
	QTextCharFormat f;
	f.setFontFamily(family);
	applyCharFormatting(f);
}

void TextEdit::SetFontSize(int size) {
	QTextCharFormat f;
	f.setFontPointSize(size);
	applyCharFormatting(f);
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

void TextEdit::applyCharFormatting(const QTextCharFormat &newFormat, const CharFormatApplyMode mode) {
	qDebug() << "applyCharFormatting";
	QTextCursor cursor = this->textCursor();
	int selectionStart = cursor.selectionStart();
	int selectionEnd = cursor.selectionEnd();
	int fragmentStart = 0;
	int fragmentEnd = 0;
	qDebug() << "Selection: start: " << selectionStart << ", end: " << selectionEnd;
	QTextBlock block = document()->findBlock(selectionStart);
	QList<QPair<int, int> > fragments;
	QList<QPair<int, int> > hyperLinkFragments;

	// If document is empty change default font
	if (this->document()->isEmpty()) {
		QFont font = cursor.charFormat().font();
		if (newFormat.hasProperty(QTextFormat::FontPointSize)) {font.setPointSize(newFormat.fontPointSize());}
		if (newFormat.hasProperty(QTextFormat::FontFamily)) {font.setFamily(newFormat.fontFamily());}
		this->document()->setDefaultFont(font);

		// Update format for default block in empty document.
		// Sometimes first block in empty text document, even though it is empty, keeps text formatting and can have
		// font settings different from document's default font. This code removes font information from such block.
		if (block.isValid()) {
			QTextCursor blockCursor(block);
			QTextCharFormat oldFormat = blockCursor.blockCharFormat();
			oldFormat.clearProperty(QTextFormat::FontPointSize);
			oldFormat.clearProperty(QTextFormat::FontFamily);
			blockCursor.setBlockCharFormat(oldFormat);

			blockCursor.mergeBlockCharFormat(newFormat);
			blockCursor.mergeBlockFormat(QTextBlockFormat(block.blockFormat()));
		}
	}

	// If nothing is selected, set format for empty cursor
	if (selectionStart == selectionEnd) {
		if (mode == Merge) {
			if (cursor.charFormat().hasProperty(QTextFormat::FontPixelSize) && newFormat.hasProperty(QTextFormat::FontPointSize)) {
				QTextCharFormat oldFormat = cursor.charFormat();
				oldFormat.clearProperty(QTextFormat::FontPixelSize);
				cursor.setCharFormat(oldFormat);
			}
			cursor.mergeCharFormat(newFormat);
		} else {
			cursor.setCharFormat(newFormat);
		}
		setTextCursor(cursor);
		return;
	}

	// Set format for selected text
	while (true) {
		if (!block.isValid()) {break;}
		if (block.blockNumber() == -1) {
			break;
		}
		if (block.position() > selectionEnd) {break;}

		QTextBlock::iterator it;
		qDebug() << "Text block #" << block.blockNumber() << ". Text:\n" << block.text();

		for(it = block.begin(); !(it.atEnd()); ++it) {
			QTextFragment currentFragment = it.fragment();
			if (!currentFragment.isValid()) {continue;}
			if (currentFragment.charFormat().isImageFormat()) {continue;}

			int fs = currentFragment.position();
			int fe = currentFragment.position() + currentFragment.length();

			if (selectionEnd < fs || selectionStart > fe) {continue;} // skip not affected fragments

			if (selectionStart < fs) {
				fragmentStart = fs;
			} else {
				fragmentStart = selectionStart;
			}

			if (selectionEnd < fe) {
				fragmentEnd = selectionEnd;
			} else {
				fragmentEnd = fe;
			}

			if (currentFragment.charFormat().isAnchor()) {
				hyperLinkFragments.append(QPair<int, int>(fragmentStart, fragmentEnd - fragmentStart));
			} else {
				fragments.append(QPair<int, int>(fragmentStart, fragmentEnd - fragmentStart));
			}
		}

		block = block.next();
	}

	QPair<int, int> pair;
	QTextCharFormat hyperlinkFormat;
	cursor.beginEditBlock();
	foreach (pair, fragments) {
		cursor.setPosition(pair.first);
		cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, pair.second);

		if (mode == Merge) {
			// If user wants to set font point size, but text has pixel size set, first remove pixel size property
			// As fas as i understand it, pixel size has priority over point size
			if (cursor.charFormat().hasProperty(QTextFormat::FontPixelSize) && newFormat.hasProperty(QTextFormat::FontPointSize)) {
				QTextCharFormat oldFormat = cursor.charFormat();
				oldFormat.clearProperty(QTextFormat::FontPixelSize);
				cursor.setCharFormat(oldFormat);
			}
			cursor.mergeCharFormat(newFormat);
		} else {
			cursor.setCharFormat(newFormat);
		}
	}
	// Change format for hyperlinks
	QTextCharFormat newHyperlinkFormat = newFormat;
	foreach (pair, hyperLinkFragments) {
		cursor.setPosition(pair.first);
		cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, pair.second);

		hyperlinkFormat = cursor.charFormat();
		newHyperlinkFormat.setAnchor(hyperlinkFormat.isAnchor());
		newHyperlinkFormat.setAnchorHref(hyperlinkFormat.anchorHref());
		newHyperlinkFormat.setAnchorName(hyperlinkFormat.anchorName());
		newHyperlinkFormat.setAnchorNames(hyperlinkFormat.anchorNames());
		newHyperlinkFormat.setUnderlineStyle(hyperlinkFormat.underlineStyle());
		newHyperlinkFormat.setUnderlineColor(hyperlinkFormat.underlineColor());
		newHyperlinkFormat.setForeground(hyperlinkFormat.foreground());

		if (mode == Merge) {
			// If user wants to set font point size, but text has pixel size set, first remove pixel size property
			if (cursor.charFormat().hasProperty(QTextFormat::FontPixelSize) && newHyperlinkFormat.hasProperty(QTextFormat::FontPointSize)) {
				QTextCharFormat oldFormat = cursor.charFormat();
				oldFormat.clearProperty(QTextFormat::FontPixelSize);
				cursor.setCharFormat(oldFormat);
			}
			cursor.mergeCharFormat(newHyperlinkFormat);
		} else {
			cursor.setCharFormat(newHyperlinkFormat);
		}
	}
	cursor.endEditBlock();
}

void TextEdit::sl_AnchorTooltipTimer_Timeout() {
	QString href = anchorAt(mapFromGlobal(QCursor::pos()));
	if (!href.isEmpty()) {
		QToolTip::showText(QCursor::pos(), href + "\n" + "Ctrl+Click to go", this);
	}
}

void TextEdit::sl_CopyCurrentFormat(bool copy) {
	if ((QApplication::keyboardModifiers() & Qt::ControlModifier) == Qt::ControlModifier) {
		if (copy) {
			blockFormatToCopy = textCursor().blockFormat();
		} else {
			blockFormatToCopy = QTextFormat(QTextFormat::InvalidFormat);
		}
	}
	if (copy) {
		charFormatToCopy = textCursor().charFormat();
	} else {
		charFormatToCopy = QTextFormat(QTextFormat::InvalidFormat);
	}
}

void TextEdit::sl_SaveImageAction_Triggered() {
	QAction* action = qobject_cast<QAction*>(QObject::sender());
	if (action == nullptr) {return;}

	QPoint pos = action->data().toPoint();
	QTextFragment fragment = findFragmentAtPos(pos);
	if (fragment.isValid() && fragment.charFormat().isImageFormat()) {
		QTextImageFormat format = fragment.charFormat().toImageFormat();
		const QString imageName = QString(format.name().toUtf8());

		const TextDocument* textDocument = dynamic_cast<TextDocument*>(document());
		CachedImageFile* image = textDocument->GetResourceImage(imageName);
		if (image == nullptr) {return;}

		QString filename = QFileDialog::getSaveFileName(this, "Select a name", image->GetFileName(),
			"");
		if (filename.isNull()) {return;}

		image->Save(filename);
	}
}

void TextEdit::sl_ResizeImageAction_Triggered() {
	QAction* action = qobject_cast<QAction*>(QObject::sender());
	if (action == nullptr) {return;}

	QPoint pos = action->data().toPoint();
	QTextFragment fragment = findFragmentAtPos(pos);
	if (!fragment.isValid() || !fragment.charFormat().isImageFormat()) {return;}

	QTextImageFormat format = fragment.charFormat().toImageFormat();

	const QString imageName = QString(format.name().toUtf8());

	const TextDocument* textDocument = dynamic_cast<TextDocument*>(document());
	CachedImageFile* image = textDocument->GetResourceImage(imageName);
	if (image == nullptr) {return;}

	SizeEditWidget resizeWidget(this);
	QSize currentImageSize = QSize();
	if (format.hasProperty(QTextFormat::ImageWidth) &&
			format.hasProperty(QTextFormat::ImageHeight)) {
		currentImageSize = QSize((int)format.width(), (int)format.height());
	}

	resizeWidget.SetData(currentImageSize, image->ImageSize());
	resizeWidget.exec();

	if (resizeWidget.result() != QDialog::Accepted) {return;}

	QSize newSize = resizeWidget.NewSize;
	if (newSize.isValid() && newSize != image->ImageSize()) {
		format.setWidth(newSize.width());
		format.setHeight(newSize.height());
	} else {
		format.clearProperty(QTextFormat::ImageWidth);
		format.clearProperty(QTextFormat::ImageHeight);
	}

	QTextCursor tempCursor(textCursor());
	tempCursor.beginEditBlock();
	tempCursor.setPosition(fragment.position());
	tempCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, fragment.length());
	tempCursor.setCharFormat(format);
	tempCursor.endEditBlock();
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
