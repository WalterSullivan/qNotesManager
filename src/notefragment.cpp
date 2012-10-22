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

#include "notefragment.h"

#include <QtGlobal>
#include <QMetaType>

using namespace qNotesManager;

int NoteFragment::metaTypeID = 0;

NoteFragment::NoteFragment() :
		NotePrt(0),
		Type(CaptionFragment),
		Start(-1),
		Length(0),
		Sample(QString()),
		MatchStart(-1),
		MatchLength(0) {
	if (NoteFragment::metaTypeID == 0) {
		NoteFragment::metaTypeID = qRegisterMetaType<NoteFragment>("NoteFragment");
	}
}

NoteFragment::NoteFragment(const Note* n, FragmentType t, int s, int l,
						   QString sample, int ss, int sl) :
		NotePrt(n),
		Type(t),
		Start(s),
		Length(l),
		Sample(sample),
		MatchStart(ss),
		MatchLength(sl)
{
	if (NoteFragment::metaTypeID == 0) {
		NoteFragment::metaTypeID = qRegisterMetaType<NoteFragment>("NoteFragment");
	}
	Q_ASSERT(s >= 0);
	Q_ASSERT(l >= 0);
	Q_ASSERT(n != 0);

}

NoteFragment::NoteFragment(const NoteFragment& f) :
		NotePrt(f.NotePrt),
		Type(f.Type),
		Start(f.Start),
		Length(f.Length),
		Sample(f.Sample),
		MatchStart(f.MatchStart),
		MatchLength(f.MatchLength) {

}


