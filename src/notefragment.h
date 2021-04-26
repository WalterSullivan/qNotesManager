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

#ifndef NOTEFRAGMENT_H
#define NOTEFRAGMENT_H

#include <QString>
#include <QMetaType>


namespace qNotesManager {
	class Note;

	class NoteFragment {
	private:
		static int metaTypeID;

	public:
		enum FragmentType {
			CaptionFragment,
			AuthorFragment,
			SourceFragment,
			CommentFragment,
			TextFragment
		};

		NoteFragment();
		NoteFragment(const Note* n, FragmentType t, int s, int l,
					 QString sample = "", int ss = 0, int sl = 0);
		NoteFragment(const NoteFragment&);
		~NoteFragment() {};

		const Note* const NotePtr;
		const FragmentType Type;
		const int Start;
		const int Length;
		const QString Sample;
		const int MatchStart;
		const int MatchLength;
	};
}

#endif // NOTEFRAGMENT_H
