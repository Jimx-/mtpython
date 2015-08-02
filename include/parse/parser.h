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
			mtpython::tree::ASTNode* suite();
			mtpython::tree::ASTNode* stmt();
			mtpython::tree::ASTNode* expr_stmt();
			mtpython::tree::ASTNode* expr();
			mtpython::tree::ASTNode* function_def();
			mtpython::tree::ASTNode* inner_if_stmt();
			mtpython::tree::ASTNode* if_stmt();
			mtpython::tree::ASTNode* for_stmt();
			mtpython::tree::ASTNode* while_stmt();
			mtpython::tree::ASTNode* del_stmt();
			mtpython::tree::ASTNode* break_stmt();
			mtpython::tree::ASTNode* continue_stmt();
			mtpython::tree::ASTNode* pass_stmt();
			mtpython::tree::ASTNode* return_stmt();
			mtpython::tree::ASTNode* raise_stmt();
			mtpython::tree::ASTNode* arguments();
			mtpython::tree::ASTNode* exprlist();
			mtpython::tree::ASTNode* testlist_comp();
			mtpython::tree::ASTNode* testlist();
			mtpython::tree::ASTNode* test();
			mtpython::tree::ASTNode* or_test();
			mtpython::tree::ASTNode* and_test();
			mtpython::tree::ASTNode* not_test();
			mtpython::tree::ASTNode* comparison();
			mtpython::tree::ASTNode* xor_expr();
			mtpython::tree::ASTNode* and_expr();
			mtpython::tree::ASTNode* shift_expr();
			mtpython::tree::ASTNode* arith_expr();
			mtpython::tree::ASTNode* term();
			mtpython::tree::ASTNode* factor();
			mtpython::tree::ASTNode* power();
			mtpython::tree::ASTNode* atom();
			mtpython::tree::ASTNode* yield_expr();
			mtpython::tree::ASTNode* name();
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