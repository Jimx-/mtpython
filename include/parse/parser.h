#ifndef _PARSER_H_
#define _PARSER_H_

#include "parse/token.h"
#include "parse/scanner.h"
#include "parse/compile_info.h"
#include "tree/nodes.h"
#include "objects/obj_space.h"
#include <string>

namespace mtpython {
	namespace parse {

		class Parser {
		protected:
			mtpython::objects::ObjSpace* space;

			mtpython::utils::SourceBuffer sb;
			mtpython::parse::Diagnostics diag;
			mtpython::parse::Scanner s;
			mtpython::parse::CompileInfo* compile_info;
	
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
			mtpython::tree::ASTNode* try_stmt();
			mtpython::tree::ASTNode* import_stmt();
			mtpython::tree::AliasNode* dotted_as_name();
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
			mtpython::tree::ASTNode* trailer(mtpython::tree::ASTNode* left);
			mtpython::tree::ASTNode* atom();
			mtpython::tree::ASTNode* parse_number();
			mtpython::tree::ASTNode* yield_expr();
			mtpython::tree::ASTNode* name();
			mtpython::tree::ASTNode* call(mtpython::tree::ASTNode* callable);
			mtpython::tree::ASTNode* argument();
			mtpython::tree::ExceptHandlerNode* excepthandler();

			objects::M_BaseObject* parsestrplus();
			objects::M_BaseObject* parsestr();
			std::string decode_unicode_utf8(const std::string& str);
			void decode_utf8(const std::string& str, std::size_t& start, std::size_t end);
		public:
			Parser(mtpython::objects::ObjSpace* space, const std::string &source, CompileInfo* info, int flags);
			~Parser();

			void read_token();
			Token last_token();
    
			mtpython::tree::ASTNode* parse();
		};
	}
}

#endif