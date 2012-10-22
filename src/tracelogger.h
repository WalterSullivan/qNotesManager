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

#ifndef TRACELOGGER_H
#define TRACELOGGER_H

namespace qNotesManager {
	class TraceLogger {
		public:
			TraceLogger(const char* fileName, const char* funcName, int lineNumber);
			void stage(const char* stage);
			~TraceLogger();

		private:
			const char* _fileName;
			const char* _funcName;
			static int Indent;
			static const int indentStep = 4;
	};
}



#ifdef DEBUG
	#ifdef ENABLE_LOG_TRACE
		#define LOG_TRACE TraceLogger logger(__FILE__, __FUNCTION__, __LINE__)
		#define LOG_STAGE(x) logger.stage(x)
	#else
		#define LOG_TRACE
		#define LOG_STAGE(x)
	#endif
#else
	#define LOG_TRACE
	#define LOG_STAGE(x)
#endif

#endif // TRACELOGGER_H
