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

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <QString>
#include <exception>

namespace qNotesManager {
	#define WHERE QString("File: ").append(__FILE__).append(", Function: ").append(__FUNCTION__).\
		append(", Line: ").append(__LINE__)

	class Exception : public std::exception {
	public:
		Exception(QString message, QString description, QString position) :
				Message(message),
				Description(description),
				Position(position) {}

		virtual ~Exception() throw() {};

		virtual const char* what() const throw() {
			return Message.toStdString().c_str();
		}

		const QString Message;
		const QString Description;
		const QString Position;
	};
}

#endif // EXCEPTION_H
