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

#include "tag.h"

#include "exception.h"
#include "boibuffer.h"

#include <QDebug>

using namespace qNotesManager;

Tag::Tag(const QString& name) : _name(name) {
	QObject::connect(&Owners, SIGNAL(sg_ItemAboutToBeAdded(Note*)),
					 this, SIGNAL(sg_OwnerAboutToBeAdded(Note*)));
	QObject::connect(&Owners, SIGNAL(sg_ItemAdded(Note*)),
					 this, SIGNAL(sg_OwnerAdded(Note*)));
	QObject::connect(&Owners, SIGNAL(sg_ItemAboutToBeRemoved(Note*)),
					 this, SIGNAL(sg_OwnerAboutToBeRemoved(Note*)));
	QObject::connect(&Owners, SIGNAL(sg_ItemRemoved(Note*)),
					 this, SIGNAL(sg_OwnerRemoved(Note*)));
	QObject::connect(&Owners, SIGNAL(sg_AboutToBeCleared()),
					 this, SIGNAL(sg_OwnersAboutToBeRemoved()));
	QObject::connect(&Owners, SIGNAL(sg_Cleared()),
					 this, SIGNAL(sg_OwnersRemoved()));
}

QString Tag::GetName() const {
	return _name;
}

void Tag::Serialize(const int version, BOIBuffer& stream) const {
	(void)version;

	const QByteArray w_nameArray = _name.toUtf8();
	const quint32 w_nameSize = w_nameArray.size();
	const quint32 w_itemSize = w_nameSize + sizeof(w_nameSize);

	qint64 result = 0;
	result = stream.write(w_itemSize);
	result = stream.write(w_nameSize);
	result = stream.write(w_nameArray.constData(), w_nameSize);
}

/* static */
Tag* Tag::Deserialize(const int version, BOIBuffer& stream) {
	(void)version;

	qint64 result = 0;

	quint32 r_itemSize = 0;
	result = stream.read(r_itemSize);

	const qint64 streamStartPos = stream.pos();

	quint32 r_nameSize = 0;
	result = stream.read(r_nameSize);

	QByteArray r_nameArray(r_nameSize, 0x0);
	result = stream.read(r_nameArray.data(), r_nameSize);

	const quint32 bytesToSkip = r_itemSize - (stream.pos() - streamStartPos);

	if (bytesToSkip != 0) {
		stream.seek(stream.pos() + bytesToSkip); // If chunck has more data in case of newer file version.
	}

	return new Tag(r_nameArray);
}
