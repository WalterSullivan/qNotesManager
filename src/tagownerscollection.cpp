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

#include "tagownerscollection.h"



using namespace qNotesManager;

TagOwnersCollection::TagOwnersCollection() {
}

void TagOwnersCollection::Add(Note* note) {
	Q_ASSERT(note != 0);
	Q_ASSERT(!owners.contains(note));

	emit sg_ItemAboutToBeAdded(note);
	owners.append(note);
	emit sg_ItemAdded(note);
}

void TagOwnersCollection::Remove(Note* note) {
	Q_ASSERT(note != 0);
	Q_ASSERT(owners.contains(note));

	emit sg_ItemAboutToBeRemoved(note);
	owners.removeAll(note);
	emit sg_ItemRemoved(note);
}

bool TagOwnersCollection::Contains(Note* note) const {
	Q_ASSERT(note != 0);

	return owners.contains(note);
}

void TagOwnersCollection::Clear() {
	emit sg_AboutToBeCleared();

	owners.clear();

	emit sg_Cleared();
}

int TagOwnersCollection::Count() const {
	return owners.count();
}

Note* TagOwnersCollection::ItemAt(int index) const {
	Q_ASSERT(index >= 0 && index < owners.count());

	return owners.at(index);
}
