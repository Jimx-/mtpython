/* LL(1) Python parse */
#include <iostream>

#include "parse/parser.h"

using namespace mtpython::parse;
using namespace mtpython::tree;

Parser::Parser(const std::string& filename): sb(filename), diag(sb), s(sb, diag)
{
	init_res_words(s);
}

Parser::~Parser()
{

}

void Parser::read_token()
{
    cur_tok = s.get_token();
}
 
Token Parser::last_token()
{
    return cur_tok;   
}

void Parser::match(Token expected)
{
	if (cur_tok == expected) {
        cur_tok = s.get_token();
    }
	else {
		diag.error(s.get_line(), s.get_col(), "unexpected token: \"" + tok2str(cur_tok) 
										+ "\", expected \"" + tok2str(expected) + "\"");
		/* TODO: Throw SyntaxError */
	}
}

void Parser::init_res_words(Scanner& scn)
{
	scn.add_res_word("def", TOK_DEF);
	scn.add_res_word("else", TOK_ELSE);
	scn.add_res_word("elif", TOK_ELIF);
	scn.add_res_word("for", TOK_FOR);
	scn.add_res_word("in", TOK_IN);
	scn.add_res_word("if", TOK_IF);
	scn.add_res_word("while", TOK_WHILE);
	scn.add_res_word("try", TOK_TRY);
	scn.add_res_word("except", TOK_EXCEPT);
	scn.add_res_word("finally", TOK_FINALLY);
	scn.add_res_word("class", TOK_CLASS);
	scn.add_res_word("True", TOK_TRUE);
	scn.add_res_word("False", TOK_FALSE);
	scn.add_res_word("None", TOK_NONE);
	scn.add_res_word("return", TOK_RETURN);
	scn.add_res_word("break", TOK_BREAK);
	scn.add_res_word("continue", TOK_CONTINUE);
}

/*
	mod = Module(stmt* body)
	    | Interactive(stmt* body)
	    | Expression(expr body)

	    -- not really an actual node but useful in Jython's typesystem.
	    | Suite(stmt* body)
*/
ASTNode* Parser::mod()
{
	ASTNode* node = NULL;
	switch (cur_tok) {
	case TOK_FILE_INPUT:
		match(cur_tok);
		node = module();
		break;
	default:
		diag.error(s.get_line(), s.get_col(), "unknown source type");
		break;
	}

	return node;
}

ASTNode* Parser::module()
{
	ModuleNode* module = new ModuleNode(s.get_line());

	ASTNode* body = stmt();
	if (cur_tok == TOK_SEMICOLON) match(TOK_SEMICOLON);
	ASTNode* tn = body;
	while (cur_tok != TOK_EOF) {
		ASTNode* n = stmt();
		if (cur_tok == TOK_SEMICOLON) match(TOK_SEMICOLON);
		if (n != NULL) {
			if (tn == NULL)
				body = tn = n;
			else {
				tn->set_sibling(n);
				tn = n;
			}
		}
	}

	module->set_body(body);

	match(TOK_EOF);
	return module;
}

/* 	stmt = FunctionDef(identifier name, arguments args, 
                            stmt* body, expr* decorator_list)
	      | ClassDef(identifier name, expr* bases, stmt* body, expr* decorator_list)
	      | Return(expr? value)

	      | Delete(expr* targets)
	      | Assign(expr* targets, expr value)
	      | AugAssign(expr target, operator op, expr value)

	      -- not sure if bool is allowed, can always use int
 	      | Print(expr? dest, expr* values, bool nl)

	      -- use 'orelse' because else is a keyword in target languages
	      | For(expr target, expr iter, stmt* body, stmt* orelse)
	      | While(expr test, stmt* body, stmt* orelse)
	      | If(expr test, stmt* body, stmt* orelse)
	      | With(expr context_expr, expr? optional_vars, stmt* body)

	      -- 'type' is a bad name
	      | Raise(expr? type, expr? inst, expr? tback)
	      | TryExcept(stmt* body, excepthandler* handlers, stmt* orelse)
	      | TryFinally(stmt* body, stmt* finalbody)
	      | Assert(expr test, expr? msg)

	      | Import(alias* names)
	      | ImportFrom(identifier? module, alias* names, int? level)

	      -- Doesn't capture requirement that locals must be
	      -- defined if globals is
	      -- still supports use as a function!
	      | Exec(expr body, expr? globals, expr? locals)

	      | Global(identifier* names)
	      | Expr(expr value)
	      | Pass | Break | Continue

	      -- XXX Jython will be different
	      -- col_offset is the byte offset in the utf8 string the parser uses
	      attributes (int lineno, int col_offset)

	      -- BoolOp() can use left & right?
*/
ASTNode* Parser::stmt()
{
	ASTNode* node = NULL;

	switch (cur_tok) {
	case TOK_DEF:
		node = function_def();
		break;
	default:
		break;
	}

	return node;
}

ASTNode* Parser::function_def()
{
	std::cout << "FunctionDef\n";
	match(TOK_DEF);
}

/* Parse the source, return a mod tree */
ASTNode* Parser::parse()
{
	ASTNode* syntax_tree;
	cur_tok = s.get_token();
	syntax_tree = mod();
	if (cur_tok != TOK_EOF)
		diag.warning(s.get_line(), s.get_col(), "last token should be \"EOF\"");
	return syntax_tree;
}
