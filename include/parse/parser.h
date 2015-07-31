#ifndef _PARSER_H_
#define _PARSER_H_

#include "parse/token.h"
#include "parse/scanner.h"
#include "tree/nodes.h"
#include <string>

namespace mtpython {
	namespace parse {

		class Parser {
		protected:
			mtpython::utils::SourceBuffer sb;
			mtpython::parse::Diagnostics diag;
			mtpython::parse::Scanner s;
	
			Token cur_tok;
			std::string srcfile;

			void init_res_words(Scanner& scn);
			void match(Token expected);

			mtpython::tree::ASTNode* mod();
			mtpython::tree::ASTNode* module();
			mtpython::tree::ASTNode* stmt();
			mtpython::tree::ASTNode* function_def();
		public:
			Parser(const std::string& filename);
			~Parser();

			void read_token();
			Token last_token();
    
			mtpython::tree::ASTNode* parse();
		};
	}
}

#endif