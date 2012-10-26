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

#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>

#define WARNING(msg) qWarning("%s\nIn: %s ( %s(%i) )", msg, __FUNCTION__, __FILE__,__LINE__);
#define FATAL(msg) qFatal("%s\nIn: %s ( %s(%i) )", msg, __FUNCTION__, __FILE__,__LINE__);

#define WARNINGIF(exp, msg, ret) if (!(exp)) {qWarning("Warning: %s\nIn: %s ( %s(%i) )", msg, __FUNCTION__, __FILE__,__LINE__); return ret;}

#endif // COMMON_H
