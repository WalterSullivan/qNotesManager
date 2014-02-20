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

#ifndef NOTE_H
#define NOTE_H

#include "abstractfolderitem.h"
#include "notetagscollection.h"

#include <QObject>
#include <QDateTime>
#include <QIcon>
#include <QReadWriteLock>
#include <QTimer>
#include <QHash>

namespace qNotesManager {
	class Tag;
	class TextDocument;
	class CachedImageFile;

	class Note : public AbstractFolderItem {
	Q_OBJECT
	friend class Serializer;
	private:
		QString					name;
		mutable QString			text;				// Note's plain text (for text search)
		QDateTime				creationDate;
		QDateTime				modificationDate;
		QDateTime				textDate;
		QString					iconID;
		QString					author;
		QString					source;
		QString					comment;
		const QColor			defaultForeColor;
		const QColor			defaultBackColor;
		QColor					nameForeColor;
		QColor					nameBackColor;
		bool					locked;
		mutable TextDocument*	document;
		mutable QReadWriteLock	lock;

		QTimer textUpdateTimer;
		mutable QString cachedHtml;
		mutable bool textDocumentInitialized;

		void onChange();
		void initTextDocument() const;

	public:
		explicit Note(QString name = QString());
		~Note();

		QString GetName() const;
		void SetName (QString s);

		QDateTime GetCreationDate() const;
		QDateTime GetModificationDate() const;

		QPixmap GetIcon() const;
		void SetIconID(QString id);
		QString GetIconID() const;

		QString GetText() const;
		void SetText(QString);
		void SetHtml(QString);

		QColor GetNameForeColor() const;
		void SetNameForeColor(QColor);

		QColor GetNameBackColor() const;
		void SetNameBackColor(QColor);

		QColor GetDefaultForeColor() const;
		QColor GetDefaultBackColor() const;

		bool IsLocked() const;
		void SetLocked(bool);

		QString GetAuthor() const;
		void SetAuthor(QString a);

		QString GetSource() const;
		void SetSource(QString s);

		QDateTime GetTextCreationDate() const;
		void SetTextCreationDate(QDateTime d);

		QString GetComment() const;
		void SetComment(QString c);

		TextDocument* GetTextDocument();
		bool TextDocumentInitialized() const;

		NoteTagsCollection Tags;
		bool IsTagsListInitializationInProgress;

	signals:
		void sg_VisualPropertiesChanged(); // emitted when name or icon or other properties has been changed
		void sg_PropertyChanged();
		void sg_DataChanged(); // emitted when any data has been changed

		void sg_ModifyDateChanged();
		void sg_TextDateChanged();

		void sg_TextChanged();

		void sg_TagAboutToBeAdded(Tag*);
		void sg_TagAdded(Tag*);
		void sg_TagAboutToBeRemoved(Tag*);
		void sg_TagRemoved(Tag*);


	private slots:
		void sl_DocumentChanged();
		void sl_TagsCollectionModified(Tag*);
		void sl_TextUpdateTimer_Timeout();

	};
}

#endif // NOTE_H
