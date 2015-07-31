#ifndef _DIAGNOSTICS_H_
#define _DIAGNOSTICS_H_

#include "utils/source_buffer.h"

namespace mtpython {
	namespace parse {

		class Diagnostics {
		private:
			utils::SourceBuffer source;
		public:
			Diagnostics(const utils::SourceBuffer& sb) : source(sb) {}
			
			void warning(int line, int col, const std::string& warning_msg);
			void error(int line, int col, const std::string& error_msg);
		};
	}
}

#endif
