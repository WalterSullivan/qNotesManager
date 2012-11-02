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

#include "searchmodelitem.h"

#include "note.h"
#include "global.h"

#include <QBrush>

using namespace qNotesManager;

SearchModelItem::SearchModelItem(const NoteFragment& f) :
		BaseModelItem(BaseModelItem::SearchResult),
		fragment(f) {
	expired = false;
	if (!fragment.NotePrt) {
		WARNING("Null pointer recieved");
	} else {
		QObject::connect(fragment.NotePrt, SIGNAL(sg_TextChanged()),
						 this, SLOT(sl_Note_TextChanged()));
	}
}

/* virtual */
QVariant SearchModelItem::data(int role) const {
	if (role == Qt::DisplayRole) {
		return fragment.Sample;
	} else if (role == HighlightStartRole) {
		return fragment.MatchStart;
	} else if (role == HightlightLengthRole) {
		return fragment.MatchLength;
	} else if (role == Qt::DecorationRole) {
		switch(fragment.Type) {
			case NoteFragment::CaptionFragment:
				return QPixmap(":/gui/edit");
				break;
			case NoteFragment::AuthorFragment:
				return QPixmap(":/gui/user");
				break;
			case NoteFragment::SourceFragment:
				return QPixmap(":/gui/globe-green");
				break;
			case NoteFragment::CommentFragment:
				return QPixmap(":/gui/balloon-box");
				break;
			case NoteFragment::TextFragment:
				return QPixmap(":/gui/edit-lipsum");
				break;
			default:
				WARNING("Unknown fragment type");
				return QVariant();
				break;
		}
	}
	return QVariant();
}

/* virtual */
Qt::ItemFlags SearchModelItem::flags () const {
	Qt::ItemFlags f = BaseModelItem::flags();
	if (expired && (f & Qt::ItemIsEnabled)) {
		f ^= Qt::ItemIsEnabled;
	}

	return f;
}

NoteFragment SearchModelItem::Fragment() const {
	return fragment;
}

void SearchModelItem::sl_Note_TextChanged() {
	expired = true;
	emit sg_DataChanged(this);
}
