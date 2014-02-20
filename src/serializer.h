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

#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <QObject>
#include <QSemaphore>

#include "document.h"
#include "boibuffer.h"

namespace qNotesManager {
	class Serializer : public QObject {
	Q_OBJECT
		enum Operation {Unknown, Loading, Saving};

		Operation operation;
		Document* doc;
		QString filename;
		quint16 saveVersion;

		void loadDocument();
		void loadDocument_v1(BOIBuffer&);

		Note* loadNote_v1(BOIBuffer&);

		void saveDocument();
		void saveDocument_v1();

		void saveNote_v1(const Note*, BOIBuffer&);

		void sendProgressSignal(BOIBuffer*);

	public:
		explicit Serializer();

		static const quint16 lastSupportedSpecificationVersion = 0x0001;
		static const quint16 actualSpecificationVersion = lastSupportedSpecificationVersion;

		void Load(Document* d, const QString& fileNameToLoad);
		void Save(Document* d, const QString& fileNameToSave, quint16 version);

	signals:
		void sg_LoadingStarted();
		void sg_LoadingProgress(int);
		void sg_LoadingPartiallyFinished();
		void sg_LoadingFinished();
		void sg_LoadingFailed(QString errorString);
		void sg_LoadingAborted();

		void sg_SavingStarted();
		void sg_SavingProgress(int);
		void sg_SavingFinished();
		void sg_SavingFailed(QString errorString);
		void sg_SavingAborted();

		void sg_PasswordRequired(QSemaphore*, QString*, bool);
		void sg_ConfirmationRequest(QSemaphore*, QString, bool*);
		void sg_Message(QString);

		void sg_finished();

	public slots:
		void sl_start();
	};
}

#endif // SERIALIZER_H
