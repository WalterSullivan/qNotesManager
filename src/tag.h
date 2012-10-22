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

#ifndef TAG_H
#define TAG_H

#include <QObject>
#include "tagownerscollection.h"

namespace qNotesManager {
	class Note;
	class BOIBuffer;

	class Tag : public QObject {
	Q_OBJECT
	private:
		const QString _name;
		Tag(const Tag& t) {}

	public:
		explicit Tag(const QString& name);
		QString GetName() const;

		void Serialize(const int version, BOIBuffer& stream) const;
		static Tag* Deserialize(const int version, BOIBuffer& stream);

		TagOwnersCollection Owners;

	signals:
		void sg_OwnerAboutToBeAdded(Note*);
		void sg_OwnerAdded(Note*);
		void sg_OwnerAboutToBeRemoved(Note*);
		void sg_OwnerRemoved(Note*);
		void sg_OwnersAboutToBeRemoved();
		void sg_OwnersRemoved();
	};
}

#endif // TAG_H
