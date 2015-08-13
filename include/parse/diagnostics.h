#ifndef _DIAGNOSTICS_H_
#define _DIAGNOSTICS_H_

#include "parse/compile_info.h"
#include "utils/source_buffer.h"

namespace mtpython {
	namespace parse {

		class Diagnostics {
		private:
			utils::SourceBuffer* source;
			CompileInfo* info;
		public:
			Diagnostics(CompileInfo* info, utils::SourceBuffer* source) : info(info), source(source) {}
			
			void warning(int line, int col, const std::string& warning_msg);
			void error(int line, int col, const std::string& error_msg);
		};
	}
}

#endif
