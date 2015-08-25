/* LL(1) Python parse */
#include <iostream>

#include "parse/parser.h"

using namespace mtpython::parse;
using namespace mtpython::tree;

Parser::Parser(mtpython::objects::ObjSpace* space, const std::string& source, CompileInfo* info): sb(source, info->get_type()), diag(info, &sb), s(&sb, &diag)
{
	this->space = space;
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
	scn.add_res_word("and", TOK_AND);
	scn.add_res_word("or", TOK_OR);
	scn.add_res_word("not", TOK_NOT);
	scn.add_res_word("lambda", TOK_LAMBDA);
	scn.add_res_word("del", TOK_DEL);
	scn.add_res_word("yield", TOK_YIELD);
	scn.add_res_word("from", TOK_FROM);
	scn.add_res_word("raise", TOK_RAISE);
	scn.add_res_word("pass", TOK_PASS);
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
	ASTNode* node = nullptr;
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
	if (cur_tok != TOK_EOF) match(TOK_NEWLINE);
	ASTNode* tn = body;
	while (cur_tok != TOK_EOF) {
		ASTNode* n = stmt();
		if (cur_tok == TOK_SEMICOLON) match(TOK_SEMICOLON);
		if (cur_tok != TOK_EOF) match(TOK_NEWLINE);
		if (n != nullptr) {
			if (tn == nullptr)
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

/* suite: simple_stmt | NEWLINE INDENT stmt+ DEDENT */
ASTNode* Parser::suite()
{
	ASTNode* node;

	if (cur_tok == TOK_NEWLINE) {
		match(TOK_NEWLINE);
		match(TOK_INDENT);

		node = stmt();
		if (cur_tok == TOK_SEMICOLON) match(TOK_SEMICOLON);
		if (cur_tok != TOK_DEDENT) match(TOK_NEWLINE);
		ASTNode* tn = node;
		while (cur_tok != TOK_DEDENT) {
			ASTNode* n = stmt();
			if (cur_tok == TOK_SEMICOLON) match(TOK_SEMICOLON);
			if (cur_tok != TOK_DEDENT) match(TOK_NEWLINE);
			if (n != nullptr) {
				if (tn == nullptr)
					node = tn = n;
				else {
					tn->set_sibling(n);
					tn = n;
				}
			}
		}

		match(TOK_DEDENT);
	} else {
		node = stmt();
	}

	return node;
}

ASTNode* Parser::stmt()
{
	ASTNode* node = nullptr;

	switch (cur_tok) {
	case TOK_DEF:
		node = function_def();
		break;
	case TOK_IF:
		node = if_stmt();
		break;
	case TOK_FOR:
		node = for_stmt();
		break;
	case TOK_WHILE:
		node = while_stmt();
		break;
	case TOK_BREAK:
		node = break_stmt();
		break;
	case TOK_CONTINUE:
		node = continue_stmt();
		break;
	case TOK_RETURN:
		node = return_stmt();
		break;
	case TOK_DEL:
		node = del_stmt();
		break;
	case TOK_RAISE:
		node = raise_stmt();
		break;
	case TOK_YIELD:
		node = yield_expr();
		break;
	case TOK_PASS:
		node = pass_stmt();
		break;
	case TOK_IDENT:
	case TOK_INTLITERAL:
		node = expr_stmt();
		break;
	case TOK_INDENT:
		diag.error(s.get_line(), s.get_col(), "unexpected indent");
		break;
	case TOK_NEWLINE:
		break;
	default:
		diag.error(s.get_line(), s.get_col(), "unexpected token " + tok2str(cur_tok));
		break;
	}

	return node;
}

ASTNode* Parser::expr_stmt()
{
	ASTNode* expression = testlist();

	if (cur_tok == TOK_EQL) {
		match(TOK_EQL);
		expression->set_context(EC_STORE);
		AssignNode* asgn = new AssignNode(s.get_line());

		if (TupleNode* tuple = dynamic_cast<TupleNode*>(expression)) {
			asgn->set_targets(tuple->get_elements());
			delete tuple;
		} else {
			asgn->push_target(expression);
		}
		asgn->set_value(testlist());
		expression = asgn;
	} else if (cur_tok == TOK_PLUSEQ || cur_tok == TOK_MINUSEQ || cur_tok == TOK_STAREQ || cur_tok == TOK_SLASHEQ) {
		if (TupleNode* tuple = dynamic_cast<TupleNode*>(expression)) {
			delete tuple;
			diag.error(s.get_line(), s.get_col(), "Illegal expression for augmented assignment");
		}

		AugAssignNode* asgn = new AugAssignNode(s.get_line());
		expression->set_context(EC_STORE);
		asgn->set_target(expression);
		asgn->set_op(tok2binop(cur_tok));
		match(cur_tok);
		asgn->set_value(testlist());

		expression = asgn;
	}

	return expression;
}

ASTNode* Parser::testlist_comp()
{
	ASTNode* elt = test();
	if (cur_tok == TOK_COMMA) {
		TupleNode* tuple = new TupleNode(s.get_line());
		tuple->push_element(elt);
		while (cur_tok == TOK_COMMA) {
			match(TOK_COMMA);
			elt = test();
			if (elt != nullptr) {
				tuple->push_element(elt);
			}
		}
		elt = tuple;
	} else if (cur_tok == TOK_FOR) {
		return nullptr;
	}

	return elt;
}

/* testlist: test (',' test)* [','] */
ASTNode* Parser::testlist()
{
	ASTNode* elt = test();
	if (cur_tok == TOK_COMMA) {
		TupleNode* tuple = new TupleNode(s.get_line());
		tuple->push_element(elt);
		while (cur_tok == TOK_COMMA) {
			match(TOK_COMMA);
			elt = test();
			if (elt != nullptr) {
				tuple->push_element(elt);
			}
		}
		elt = tuple;
	}

	return elt;
}

/* test: or_test ['if' or_test 'else' test] | lambdef */
ASTNode* Parser::test()
{
	ASTNode* node = nullptr;
	if (cur_tok == TOK_LAMBDA) {
		return nullptr;
	} else {
		node = or_test();
		if (!node) return node;

		if (cur_tok == TOK_IF) {
			match(TOK_IF);
			IfExpNode* if_exp = new IfExpNode(s.get_line());
			if_exp->set_body(node);
			if_exp->set_test(or_test());
			match(TOK_ELSE);
			if_exp->set_orelse(test());
			return if_exp;
		}
	}
	return node;
}

/* or_test: and_test ('or' and_test)* */
ASTNode* Parser::or_test()
{
    ASTNode* node = and_test();
    BinOpNode* p = nullptr;
    while (cur_tok == TOK_OR) {
        p = new BinOpNode(s.get_line());
        if (p != nullptr) {
            p->set_left(node);     
            p->set_op(OP_OR);  
            match(cur_tok);
        	p->set_right(and_test());         
        	node = p; 
        }
    }
    return node;
}

/* and_test: not_test ('and' not_test)* */
ASTNode* Parser::and_test()
{
    ASTNode* node = not_test();
    BinOpNode* p = nullptr;
    while (cur_tok == TOK_AND) {
        p = new BinOpNode(s.get_line());
        if (p != nullptr) {
            p->set_left(node);     
            p->set_op(OP_AND);  
            match(cur_tok);
        	p->set_right(not_test());         
        	node = p; 
        }
    }
    return node;
}

/* not_test: 'not' not_test | comparison */
ASTNode* Parser::not_test()
{
	ASTNode* node = nullptr;
    UnaryOpNode* tmp = nullptr;

    if (cur_tok == TOK_NOT) {
    	tmp = new UnaryOpNode(s.get_line());
    	tmp->set_op(OP_NOT);
    	tmp->set_operand(not_test());
    	return tmp;
    } else {
    	node = comparison();
    }

    return node;
}

/* comparison: expr (comp_op expr)* */
ASTNode* Parser::comparison()
{
	ASTNode* node = expr();
    CompareNode* p = new CompareNode(s.get_line());
    bool used = false;
    while ((cur_tok == TOK_EQLEQL) || (cur_tok == TOK_NEQ) || (cur_tok == TOK_LSS) || (cur_tok == TOK_GTR) || (cur_tok == TOK_LEQ) || (cur_tok == TOK_GEQ)) {
        p->set_left(node);
        p->push_op(tok2cmpop(cur_tok));  
        match(cur_tok);
       	p->push_comparator(expr());
        used = true;
    }

    if (!used) {
    	delete p;
    	return node;
    }

    return p;
}

/* exprlist: expr (',' expr)* [','] */
ASTNode* Parser::exprlist()
{
	ASTNode* elt = expr();
	if (cur_tok == TOK_COMMA) {
		TupleNode* tuple = new TupleNode(s.get_line());
		tuple->push_element(elt);
		while (cur_tok == TOK_COMMA) {
			match(TOK_COMMA);
			elt = expr();
			if (elt != nullptr) {
				tuple->push_element(elt);
			}
		}
		elt = tuple;
	}

	return elt;
}

/* expr: xor_expr ('|' xor_expr)* */
ASTNode* Parser::expr()
{
    ASTNode* node = xor_expr();
    BinOpNode* p = nullptr;
    while (cur_tok == TOK_VERTBAR) {
        p = new BinOpNode(s.get_line());
        if (p != nullptr) {
            p->set_left(node);     
            p->set_op(tok2binop(cur_tok));  
            match(cur_tok);
        	p->set_right(xor_expr());         
        	node = p; 
        }
    }
    return node;
}

/* xor_expr: and_expr ('^' and_expr)* */
ASTNode* Parser::xor_expr()
{
    ASTNode* node = and_expr();
    BinOpNode* p = nullptr;
    while (cur_tok == TOK_CARPET) {
        p = new BinOpNode(s.get_line());
        if (p != nullptr) {
            p->set_left(node);     
            p->set_op(tok2binop(cur_tok));  
            match(cur_tok);
        	p->set_right(and_expr());         
        	node = p; 
        }
    }
    return node;
}

/* and_expr: shift_expr ('&' shift_expr)* */
ASTNode* Parser::and_expr()
{
    ASTNode* node = shift_expr();
    BinOpNode* p = nullptr;
    while (cur_tok == TOK_AMP) {
        p = new BinOpNode(s.get_line());
        if (p != nullptr) {
            p->set_left(node);     
            p->set_op(tok2binop(cur_tok));  
            match(cur_tok);
        	p->set_right(shift_expr());         
        	node = p; 
        }
    }
    return node;
}

/* shift_expr: arith_expr (('<<'|'>>') arith_expr)* */
ASTNode* Parser::shift_expr()
{
    ASTNode* node = arith_expr();
    BinOpNode* p = nullptr;
    if ((cur_tok == TOK_LSSLSS) || (cur_tok == TOK_GTRGTR)) {
        p = new BinOpNode(s.get_line());
        if (p != nullptr) {
            p->set_left(node);     
            p->set_op(tok2binop(cur_tok));  
            match(cur_tok);
        	p->set_right(arith_expr());         
        	node = p; 
        }
    }
    return node;
}

/* arith_expr: term (('+'|'-') term)* */
ASTNode* Parser::arith_expr()
{
    ASTNode* node = term();
    BinOpNode * p = nullptr;
    while ((cur_tok == TOK_PLUS) || (cur_tok == TOK_MINUS)) { 
        p = new BinOpNode(s.get_line()); 
        if (p != nullptr) {
            p->set_left(node);
            p->set_op(tok2binop(cur_tok));
            match(cur_tok);
            p->set_right(term());
            node = p;
        }
    }
    return node;
}

/* term: factor (('*'|'/'|'%'|'//') factor)* */
ASTNode* Parser::term()
{
    ASTNode* node = factor();
    BinOpNode * p = nullptr;
    while ((cur_tok == TOK_STAR) || (cur_tok == TOK_SLASH) || (cur_tok == TOK_PERCENT) || (cur_tok == TOK_SLASHSLASH)) { 
        p = new BinOpNode(s.get_line());  
        if (p != nullptr) {
            p->set_left(node);
            p->set_op(tok2binop(cur_tok));
            match(cur_tok);
            p->set_right(factor());
            node = p;
        }
    }
    return node;    
}

/* factor: ('+'|'-'|'~') factor | power */
ASTNode* Parser::factor()
{
    ASTNode* node = nullptr;
    UnaryOpNode* tmp = nullptr;
	Token tok = cur_tok;
	switch (cur_tok) {
	case TOK_TILDE:
	case TOK_PLUS:
	case TOK_MINUS:
		match(cur_tok);
    default:
		break;
    }
    node = power();
    switch (tok) {
	case TOK_TILDE:
	case TOK_PLUS:
	case TOK_MINUS:
		tmp = new UnaryOpNode(s.get_line());
		tmp->set_op(tok2unop(tok));
		tmp->set_operand(node);
		node = tmp;
		break;
	default:
		break;
    }
    return node;
}

/* power: atom trailer* ['**' factor] */
ASTNode* Parser::power()
{
    ASTNode* node = atom();

    while (true) {
    	ASTNode* tmp_node = trailer(node);
    	if (!tmp_node) break;
    	node = tmp_node;
    }

    BinOpNode* p = nullptr;
    while (cur_tok == TOK_STARSTAR)
    { 
        p = new BinOpNode(s.get_line());  
        if (p != nullptr) 
        {
            p->set_left(node);
            p->set_op(tok2binop(cur_tok));
            match(cur_tok);
            p->set_right(factor());
            node = p;
        }
    }
    return node;    
}

/* trailer: '(' [arglist] ')' | '[' subscriptlist ']' | '.' NAME */
ASTNode* Parser::trailer(ASTNode* left)
{
	ASTNode* node = nullptr;

	switch (cur_tok)
	{
	case TOK_LPAREN:
		return call(left);
	}

	return node;
}

/* atom: ('(' [yield_expr|testlist_comp] ')' |
       '[' [testlist_comp] ']' |
       '{' [dictorsetmaker] '}' |
       NAME | NUMBER | STRING+ | '...' | 'None' | 'True' | 'False') */
ASTNode* Parser::atom()
{
    ASTNode* node = nullptr;
    switch (cur_tok) 
    {
    case TOK_IDENT :
        node = name();
        break;
    case TOK_LPAREN :
    	match(TOK_LPAREN);
    	if (cur_tok == TOK_RPAREN) 
			node = new TupleNode(s.get_line());
    	else 
    		node = testlist_comp();
		match(TOK_RPAREN);
        break;
    case TOK_INTLITERAL:
    case TOK_LONGLITERAL:
    case TOK_FLOATLITERAL:
    case TOK_DOUBLELITERAL:
    	node = parse_number();
    	break;
    case TOK_STRINGLITERAL:
	{
		/* TODO: handle encoding, str concat */
		StringNode* str_node = new StringNode(s.get_line());
		str_node->set_value(space->wrap_str(s.get_last_string()));
		match(TOK_STRINGLITERAL);
		node = str_node;
		break;
	}
    case TOK_TRUE:
    case TOK_FALSE:
    case TOK_NONE:
         node = nullptr;
         break;
    default:
		return nullptr;
    }
    return node;
}

ASTNode* Parser::call(ASTNode* callable)
{
	CallNode* node = new CallNode(s.get_line());
	node->set_func(callable);
	match(TOK_LPAREN);

	while (cur_tok != TOK_RPAREN) {
		if (cur_tok == TOK_STAR) {

		} else if (cur_tok == TOK_STARSTAR) {

		} else {
			ASTNode* arg = argument();
			KeywordNode* keyword = dynamic_cast<KeywordNode*>(arg);
			if (keyword) {
				node->push_keyword(keyword);
			} else {
				node->push_arg(arg);
			}
		}

		if (cur_tok != TOK_RPAREN) match(TOK_COMMA);
	}

	match(TOK_RPAREN);
	return node;
}

ASTNode* Parser::argument()
{
	ASTNode* node = test();
	
	if (cur_tok == TOK_EQL) {
		KeywordNode* keyword = new KeywordNode(s.get_line());

		NameNode* name_node = dynamic_cast<NameNode*>(node);
		if (!name_node) diag.error(s.get_line(), s.get_col(), "invalid syntax");

		keyword->set_arg(name_node->get_name());
		match(TOK_EQL);
		keyword->set_value(test());

		return keyword;
	}

	return node;
}

ASTNode* Parser::parse_number()
{
	NumberNode* node = new NumberNode(s.get_line());

	if (cur_tok == TOK_INTLITERAL) {
		node->set_value(space->wrap_int(s.get_last_strnum()));
		match(TOK_INTLITERAL);
	}

	return node;
}

ASTNode* Parser::yield_expr()
{
	match(TOK_YIELD);
	if (cur_tok == TOK_FROM) {
		match(TOK_FROM);
		YieldFromNode* node = new YieldFromNode(s.get_line());
		ASTNode* expression = test();
		if (!expression) diag.error(s.get_line(), s.get_col(), "invalid syntax");
		node->set_value(expression);

		return node;
	} else {
		YieldNode* node = new YieldNode(s.get_line());
		node->set_value(testlist());

		return node;
	}

	return nullptr;
}

/* funcdef: 'def' NAME parameters ['->' test] ':' suite */
ASTNode* Parser::function_def()
{
	FunctionDefNode* node = new FunctionDefNode(s.get_line());
	match(TOK_DEF);

	node->set_name(s.get_last_word());
	match(TOK_IDENT);
	node->set_args(arguments());
	match(TOK_COLON);

	node->set_body(suite());

	return node;
}

ASTNode* Parser::inner_if_stmt()
{
	IfNode* node = new IfNode(s.get_line());

	ASTNode* cond = test();
	if (!cond) diag.error(s.get_line(), s.get_col(), "invalid syntax");
	node->set_test(cond);

	match(TOK_COLON);
	node->set_body(suite());

	if (cur_tok == TOK_ELSE) {
		match(TOK_ELSE);
		match(TOK_COLON);
		node->set_orelse(suite());
	} else if (cur_tok == TOK_ELIF) {
		match(TOK_ELIF);
		node->set_orelse(inner_if_stmt());
	}

	return node;
}

ASTNode* Parser::if_stmt()
{
	match(TOK_IF);
	return inner_if_stmt();
}

ASTNode* Parser::for_stmt()
{
	ForNode* node = new ForNode(s.get_line());

	match(TOK_FOR);
	ASTNode* target = exprlist();
	if (!target) diag.error(s.get_line(), s.get_col(), "invalid syntax");
	target->set_context(EC_STORE);
	node->set_target(target);

	match(TOK_IN);
	ASTNode* iter = testlist();
	if (!iter) diag.error(s.get_line(), s.get_col(), "invalid syntax");
	node->set_iter(iter);

	match(TOK_COLON);
	node->set_body(suite());

	if (cur_tok == TOK_ELSE) {
		match(TOK_ELSE);
		match(TOK_COLON);
		node->set_orelse(suite());
	}

	return node;
}

ASTNode* Parser::while_stmt()
{
	WhileNode* node = new WhileNode(s.get_line());

	match(TOK_WHILE);
	ASTNode* cond = test();
	if (!cond) diag.error(s.get_line(), s.get_col(), "invalid syntax");
	node->set_test(cond);

	match(TOK_COLON);
	node->set_body(suite());

	if (cur_tok == TOK_ELSE) {
		match(TOK_ELSE);
		match(TOK_COLON);
		node->set_orelse(suite());
	}

	return node;
}

/* del_stmt: 'del' exprlist */
ASTNode* Parser::del_stmt()
{
	DeleteNode* node = new DeleteNode(s.get_line());
	match(TOK_DEL);

	ASTNode* target = expr();
	if (!target) diag.error(s.get_line(), s.get_col(), "invalid syntax"); 

	target->set_context(EC_DEL);
	node->push_target(target);
	while (cur_tok == TOK_COMMA) {
		match(TOK_COMMA);
		target = expr();
		if (target) {
			target->set_context(EC_DEL);
			node->push_target(target);
		}
	}

	return node;
}

/* return_stmt: 'return' [testlist] */
ASTNode* Parser::return_stmt()
{
	ReturnNode* node = new ReturnNode(s.get_line());
	match(TOK_RETURN);
	node->set_value(testlist());
	return node;
}

/* break_stmt: 'break' */
ASTNode* Parser::break_stmt()
{
	match(TOK_BREAK);
	return new BreakNode(s.get_line());
}

/* continue_stmt: 'continue' */
ASTNode* Parser::continue_stmt()
{
	match(TOK_CONTINUE);
	return new ContinueNode(s.get_line());
}

/* pass_stmt: 'pass' */
ASTNode* Parser::pass_stmt()
{
	match(TOK_PASS);
	return new PassNode(s.get_line());
}


/* raise_stmt: 'raise' [test ['from' test]] */
ASTNode* Parser::raise_stmt()
{
	RaiseNode* node = new RaiseNode(s.get_line());
	match(TOK_RAISE);
	ASTNode* expression = test();
	if (expression) {
		node->set_exc(expression);

		if (cur_tok == TOK_FROM) {
			match(TOK_FROM);
			expression = test();
			if (!expression) diag.error(s.get_line(), s.get_col(), "invalid syntax");
			node->set_cause(expression);
		}
	}

	return node;
}

ASTNode* Parser::arguments()
{
	ArgumentsNode* node = new ArgumentsNode(s.get_line());
	ASTNode* p = nullptr;
	match(TOK_LPAREN);
	if (cur_tok != TOK_RPAREN) {
		p = expr();
		p->set_context(EC_PARAM);
		node->push_arg(p);
		while (cur_tok == TOK_COMMA) {
            match(TOK_COMMA);
			p = expr();
			p->set_context(EC_PARAM);
			node->push_arg(p);
		}
	}
	match(TOK_RPAREN);
	return node;
}

ASTNode* Parser::name()
{
	NameNode* node = new NameNode(s.get_line());
	node->set_name(s.get_last_word());
	match(TOK_IDENT);
	return node;
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
