/* LL(1) Python parse */
#include <iostream>

#include "parse/parser.h"

using namespace mtpython::parse;
using namespace mtpython::tree;
using namespace mtpython::objects;

Parser::Parser(mtpython::objects::ObjSpace* space, const std::string &source, CompileInfo* info, int flags) : sb(source, info->get_type()), diag(info, &sb), s(&sb, &diag)
{
	this->space = space;
	init_res_words(s);

	if (flags & PyCF_SOURCE_IS_UTF8) {
		info->set_encoding("utf-8");
	}

	compile_info = info;
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
	scn.add_res_word("as", TOK_AS);
	scn.add_res_word("import", TOK_IMPORT);
	scn.add_res_word("is", TOK_IS);
	scn.add_res_word("with", TOK_WITH);
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
	ASTNode* tn = body;
	while (cur_tok != TOK_EOF) {
		ASTNode* n = stmt();
		if (cur_tok == TOK_SEMICOLON) match(TOK_SEMICOLON);
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
		ASTNode* tn = node;
		while (cur_tok != TOK_DEDENT) {
			ASTNode* n = stmt();
			if (cur_tok == TOK_SEMICOLON) match(TOK_SEMICOLON);
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
	bool compound_stmt = false;

	switch (cur_tok) {
	case TOK_DEF:
		compound_stmt = true;
		node = function_def();
		break;
	case TOK_CLASS:
		compound_stmt = true;
		node = class_def();
		break;
	case TOK_IF:
		compound_stmt = true;
		node = if_stmt();
		break;
	case TOK_FOR:
		compound_stmt = true;
		node = for_stmt();
		break;
	case TOK_WHILE:
		compound_stmt = true;
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
	case TOK_TRY:
		compound_stmt = true;
		node = try_stmt();
		break;
	case TOK_WITH:
		compound_stmt = true;
		node = with_stmt();
		break;
	case TOK_IMPORT:
		node = import_stmt();
		break;
	case TOK_FROM:
		node = import_from_stmt();
		break;
	case TOK_LAMBDA:
	case TOK_IDENT:
	case TOK_INTLITERAL:
	case TOK_STRINGLITERAL:
	case TOK_NONE:
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

	if (!compound_stmt) {
		if (cur_tok != TOK_EOF && cur_tok != TOK_DEDENT) match(TOK_NEWLINE);
	}

	return node;
}

ASTNode* Parser::expr_stmt()
{
	ASTNode* expression = testlist();

	if (cur_tok == TOK_EQL) {
		expression->set_context(EC_STORE);
		AssignNode* asgn = new AssignNode(s.get_line());

		if (TupleNode* tuple = dynamic_cast<TupleNode*>(expression)) {
			asgn->set_targets(tuple->get_elements());
		} else {
			asgn->push_target(expression);
		}

		ASTNode* tgt;
		while (cur_tok == TOK_EQL) {
			match(TOK_EQL);
			tgt = testlist();
			if (cur_tok == TOK_EQL) {
				tgt->set_context(EC_STORE);
				asgn->push_target(tgt);
			}
		}

		asgn->set_value(tgt);
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
	} else {
		ExprNode* expr = new ExprNode(s.get_line());
		expr->set_value(expression);
		expression = expr;
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
		GeneratorExpNode* genexp = new GeneratorExpNode(s.get_line());
		genexp->set_elt(elt);

		while (cur_tok == TOK_FOR) {
			genexp->push_comprehension(comp_for());
		}

		elt = genexp;
	}

	return elt;
}

ASTNode* Parser::testlist_comp_list()
{
	ASTNode* elt = test();
	if (cur_tok == TOK_COMMA) {
		ListNode* list = new ListNode(s.get_line());
		list->push_element(elt);
		while (cur_tok == TOK_COMMA) {
			match(TOK_COMMA);
			elt = test();
			if (elt != nullptr) {
				list->push_element(elt);
			}
		}
		elt = list;
	} else if (cur_tok == TOK_FOR) {
		GeneratorExpNode* genexp = new GeneratorExpNode(s.get_line());
		genexp->set_elt(elt);

		while (cur_tok == TOK_FOR) {
			genexp->push_comprehension(comp_for());
		}

		elt = genexp;
	}

	return elt;
}

ASTNode* Parser::comp_if()
{
	match(TOK_IF);
	return or_test();
}

ComprehensionNode* Parser::comp_for()
{
	ComprehensionNode* node = new ComprehensionNode(s.get_line());
	match(TOK_FOR);
	ASTNode* target = exprlist();
	node->set_target(target);
	match(TOK_IN);
	ASTNode* iter = or_test();
	node->set_iter(iter);

	while (cur_tok == TOK_IF) {
		ASTNode* ifexp = comp_if();
		node->push_if(ifexp);
	}

	return node;
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
		return lambda_def();
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
		match(TOK_NOT);
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
	ASTNode* node = star_expr();
    CompareNode* p = new CompareNode(s.get_line());
    bool used = false;
    while ((cur_tok == TOK_EQLEQL) || (cur_tok == TOK_NEQ) || (cur_tok == TOK_LSS) || (cur_tok == TOK_GTR) ||
		(cur_tok == TOK_LEQ) || (cur_tok == TOK_GEQ) || (cur_tok == TOK_IN) || (cur_tok == TOK_IS) || (cur_tok == TOK_NOT)) {
		Token tok = cur_tok;
		match(cur_tok);

		if (tok == TOK_IS && cur_tok == TOK_NOT) {
			match(TOK_NOT);
			tok = TOK_IS_NOT;
		}

		if (tok == TOK_NOT) {
			if (cur_tok == TOK_IN) {
				match(TOK_IN);
				tok = TOK_NOT_IN;
			} else {
				diag.error(s.get_line(), s.get_col(), "invalid syntax");
			}
		}

        p->set_left(node);
        p->push_op(tok2cmpop(tok));
       	p->push_comparator(star_expr());
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

ASTNode* Parser::star_expr()
{
	bool starred = false;
	if (cur_tok == TOK_STAR) {
		starred = true;
		match(TOK_STAR);
	}

	ASTNode* node = expr();
	if (starred) {
		StarredNode* starred_node = new StarredNode(s.get_line());
		starred_node->set_value(node);
		starred_node->set_context(EC_LOAD);
		node = starred_node;
	}

	return node;
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
	bool has_right = false;

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

ASTNode* Parser::slice()
{
	ASTNode* node = nullptr;

	/* empty subscript */
	if (cur_tok == TOK_RSQUARE) diag.error(s.get_line(), s.get_col(), "invalid syntax");

	ASTNode* first = test();
	ASTNode* second = nullptr;
	ASTNode* third = nullptr;


	if (cur_tok == TOK_COLON) {
		match(TOK_COLON);
		second = test();
	}
	if (cur_tok == TOK_COLON) {
		match(TOK_COLON);
		third = test();
	}

	if (first && !second && !third) {
		IndexNode* index = new IndexNode(s.get_line());
		index->set_value(first);

		node = index;
	} else {
		SliceNode* slice_node = new SliceNode(s.get_line());
		slice_node->set_lower(first);
		slice_node->set_upper(second);
		slice_node->set_step(third);

		node = slice_node;
	}

	return node;
}

/* trailer: '(' [arglist] ')' | '[' subscriptlist ']' | '.' NAME */
ASTNode* Parser::trailer(ASTNode* left)
{
	ASTNode* node = nullptr;

	switch (cur_tok) {
	case TOK_LPAREN:
		node = call(left);
		break;
	case TOK_DOT:
	{
		AttributeNode* attr_node = new AttributeNode(s.get_line());
		attr_node->set_value(left);
		match(TOK_DOT);
		NameNode* as_name = dynamic_cast<NameNode*>(name());
		if (!as_name) diag.error(s.get_line(), s.get_col(), "invalid syntax");
		attr_node->set_attr(as_name->get_name());
		SAFE_DELETE(as_name);
		attr_node->set_context(EC_LOAD);
		node = attr_node;
		break;
	}
	case TOK_LSQUARE:
	{
		SubscriptNode* sub_node = new SubscriptNode(s.get_line());

		match(TOK_LSQUARE);
		ASTNode* slice_node = slice();
		sub_node->set_value(left);
		sub_node->set_slice(slice_node);
		sub_node->set_context(EC_LOAD);
		match(TOK_RSQUARE);
		node = sub_node;
		break;
	}
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
	{
		bool joining = s.get_implicit_line_joining();
		s.set_implicit_line_joining(true);
		match(TOK_LPAREN);
		if (cur_tok == TOK_RPAREN)
			node = new TupleNode(s.get_line());
		else
			node = testlist_comp();
		s.set_implicit_line_joining(joining);
		match(TOK_RPAREN);
		break;
	}
	case TOK_LSQUARE:
	{
		bool joining = s.get_implicit_line_joining();
		s.set_implicit_line_joining(true);
		match(TOK_LSQUARE);
		if (cur_tok == TOK_RSQUARE)
			node = new ListNode(s.get_line());
		else
			node = testlist_comp_list();
		s.set_implicit_line_joining(joining);
		match(TOK_RSQUARE);
		break;
	}
	case TOK_LBRACE:
	{
		bool joining = s.get_implicit_line_joining();
		s.set_implicit_line_joining(true);
		match(TOK_LBRACE);
		if (cur_tok == TOK_RBRACE)
			node = new DictNode(s.get_line());
		else {
			ASTNode* first_elt = test();
			if (cur_tok == TOK_RBRACE || cur_tok == TOK_COMMA) {	/* set */
				SetNode* set_node = new SetNode(s.get_line());
				set_node->push_element(first_elt);
				while (cur_tok == TOK_COMMA) {
					match(TOK_COMMA);
					ASTNode* elt = test();
					if (!elt) break;
					set_node->push_element(elt);
				}
				node = set_node;
			} else {

			}
		}
		s.set_implicit_line_joining(joining);
		match(TOK_RBRACE);
		break;
	}
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
		str_node->set_value(parsestrplus());
		node = str_node;
		break;
	}
    case TOK_TRUE:
	{
		ConstNode* const_node = new ConstNode(s.get_line());
		const_node->set_value(space->new_bool(true));
		match(TOK_TRUE);
		node = const_node;
		break;
	}
	case TOK_FALSE:
	{
		ConstNode* const_node = new ConstNode(s.get_line());
		const_node->set_value(space->new_bool(false));
		match(TOK_FALSE);
		node = const_node;
		break;
	}
    case TOK_NONE:
	{
		ConstNode* const_node = new ConstNode(s.get_line());
		const_node->set_value(space->wrap_None());
		match(TOK_NONE);
		node = const_node;
		break;
	}
    default:
		return nullptr;
    }
    return node;
}

M_BaseObject* Parser::parsestrplus()
{
	M_BaseObject* str = space->wrap_str(s.get_last_string());
	match(TOK_STRINGLITERAL);

	while (cur_tok == TOK_STRINGLITERAL) {
		match(TOK_STRINGLITERAL);
	}

	return str;
}

M_BaseObject* Parser::parsestr()
{
	const std::string& enc = compile_info->get_encoding();
	bool bytes_mode = true;
	std::string str = s.get_last_string();

	if (bytes_mode) {
		if (enc != "") {
			std::string decoded = decode_unicode_utf8(str);
		}
	}

	return nullptr;
}

void Parser::decode_utf8(const std::string& str, std::size_t& start, std::size_t end)
{
	std::size_t s, t;
	s = t = start;

	while (s < end && (str[s] & 0x80)) s++;
	start = s;

	std::string substr = str.substr(t, s - t);

}

std::string Parser::decode_unicode_utf8(const std::string& str)
{
	std::size_t i = 0;
	std::size_t end = str.size();
	std::string decoded;

	while (i < end) {
		if (str[i] == '\\') {
			decoded += str[i++];
			if (str[i] & 0x80) {
				decoded += "u005c";
			}
		}

		if (str[i] & 0x80) {
			decode_utf8(str, i, end);
		} else {
			decoded += str[i++];
		}
	}

	return decoded;
}

ASTNode* Parser::call(ASTNode* callable)
{
	CallNode* node = new CallNode(s.get_line());
	node->set_func(callable);

	bool joining = s.get_implicit_line_joining();
	s.set_implicit_line_joining(true);
	match(TOK_LPAREN);
	ASTNode* vararg = nullptr;
	ASTNode* kwarg = nullptr;

	while (cur_tok != TOK_RPAREN) {
		if (cur_tok == TOK_STAR) {
			match(TOK_STAR);
			vararg = test();
		} else if (cur_tok == TOK_STARSTAR) {
			match(TOK_STARSTAR);
			kwarg = test();
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

	s.set_implicit_line_joining(joining);
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
		SAFE_DELETE(name_node);
		match(TOK_EQL);
		keyword->set_value(test());

		return keyword;
	}

	return node;
}

ExceptHandlerNode* Parser::excepthandler()
{
	ExceptHandlerNode* node = new ExceptHandlerNode(s.get_line());
	match(TOK_EXCEPT);

	ASTNode* type = test();
	node->set_type(type);

	if (cur_tok == TOK_AS) {
		match(TOK_AS);
		NameNode* as_name = dynamic_cast<NameNode*>(name());
		if (!as_name) diag.error(s.get_line(), s.get_col(), "invalid syntax");
		node->set_name(as_name->get_name());
		SAFE_DELETE(as_name);
	}

	match(TOK_COLON);
	ASTNode* body = suite();
	if (!body) diag.error(s.get_line(), s.get_col(), "invalid syntax");
	node->set_body(body);

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
	match(TOK_LPAREN);
	node->set_args(arguments());
	match(TOK_RPAREN);
	match(TOK_COLON);

	node->set_body(suite());

	return node;
}

ASTNode* Parser::class_def()
{
	ClassDefNode* node = new ClassDefNode(s.get_line());
	match(TOK_CLASS);

	node->set_name(s.get_last_word());
	match(TOK_IDENT);

	if (cur_tok == TOK_LPAREN) {
		ASTNode* p;
		match(TOK_LPAREN);
		if (cur_tok != TOK_RPAREN) {
			p = test();
			if (p) node->push_base(p);

			while (cur_tok == TOK_COMMA) {
				match(TOK_COMMA);
				p = test();
				if (p) node->push_base(p);
			}
		}
		match(TOK_RPAREN);
	}

	match(TOK_COLON);
	node->set_body(suite());

	return node;
}

ASTNode* Parser::lambda_def()
{
	LambdaNode* node = new LambdaNode(s.get_line());
	match(TOK_LAMBDA);

	if (cur_tok == TOK_COLON) {
		node->set_args(new ArgumentsNode(s.get_line()));
	} else {
		node->set_args(arguments());
	}

	match(TOK_COLON);

	node->set_body(test());
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

	ASTNode* target = test();
	if (!target) diag.error(s.get_line(), s.get_col(), "invalid syntax"); 

	target->set_context(EC_DEL);
	node->push_target(target);
	while (cur_tok == TOK_COMMA) {
		match(TOK_COMMA);
		target = test();
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

/* try_stmt: ('try' ':' suite
           ((except_clause ':' suite)+
	    ['else' ':' suite]
	    ['finally' ':' suite] |
	   'finally' ':' suite)) */
ASTNode* Parser::try_stmt()
{
	TryNode* node = new TryNode(s.get_line());

	match(TOK_TRY);
	match(TOK_COLON);

	ASTNode* body = suite();
	if (!body) diag.error(s.get_line(), s.get_col(), "invalid syntax");
	node->set_body(body);

	while (cur_tok == TOK_EXCEPT) {
		ExceptHandlerNode* except_handler = excepthandler();
		node->push_handler(except_handler);
	}

	if (cur_tok == TOK_ELSE) {
		match(TOK_ELSE);
		match(TOK_COLON);

		ASTNode* orelse = suite();
		if (!orelse) diag.error(s.get_line(), s.get_col(), "invalid syntax");
		node->set_orelse(orelse);
	}

	if (cur_tok == TOK_FINALLY) {
		match(TOK_FINALLY);
		match(TOK_COLON);

		ASTNode* finalbody = suite();
		if (!finalbody) diag.error(s.get_line(), s.get_col(), "invalid syntax");
		node->set_finalbody(finalbody);
	}

	return node;
}

ASTNode* Parser::with_stmt()
{
	WithNode* node = new WithNode(s.get_line());
	match(TOK_WITH);

	ASTNode* context_expr = test();
	ASTNode* opt_vars = nullptr;
	if (cur_tok == TOK_AS) {
		match(TOK_AS);
		opt_vars = expr();
	}

	WithItemNode* with_item = new WithItemNode(s.get_line());
	with_item->set_context_expr(context_expr);
	with_item->set_optional_vars(opt_vars);
	node->push_item(with_item);

	while (cur_tok == TOK_COMMA) {
		match(TOK_COMMA);

		context_expr = test();
		opt_vars = nullptr;
		if (cur_tok == TOK_AS) {
			match(TOK_AS);
			opt_vars = expr();
		}

		with_item = new WithItemNode(s.get_line());
		with_item->set_context_expr(context_expr);
		with_item->set_optional_vars(opt_vars);
		node->push_item(with_item);
	}

	match(TOK_COLON);
	node->set_body(suite());

	return node;
}

ASTNode* Parser::import_stmt()
{
	ImportNode* node = new ImportNode(s.get_line());
	match(TOK_IMPORT);
	node->push_name(dotted_as_name());

	while (cur_tok == TOK_COMMA) {
		match(TOK_COMMA);
		node->push_name(dotted_as_name());
	}

	return node;
}

ASTNode* Parser::import_from_stmt()
{
	ImportFromNode* node = new ImportFromNode(s.get_line());

	match(TOK_FROM);

	std::string modname;
	int dot_count = 0;
	while (true) {
		if (cur_tok == TOK_IDENT) {
			modname = dotted_name();
			break;
		} else if (cur_tok == TOK_ELLIPSIS) {
			dot_count += 2;
			match(TOK_ELLIPSIS);
		} else if (cur_tok != TOK_DOT) {
			break;
		}

		match(TOK_DOT);
		dot_count++;
	}

	match(TOK_IMPORT);
	bool star_import = false;
	bool lparen = false;
	bool joining = s.get_implicit_line_joining();

	if (cur_tok == TOK_STAR) {
		star_import = true;
		match(TOK_STAR);
	} else if (cur_tok == TOK_LPAREN) {
		lparen = true;
		s.set_implicit_line_joining(true);
		match(TOK_LPAREN);
	}

	node->set_module(modname);

	if (!star_import) {
		node->push_name(dotted_as_name());

		while (cur_tok == TOK_COMMA) {
			match(TOK_COMMA);
			node->push_name(dotted_as_name());
		}
	} else {
		AliasNode* star_name = new AliasNode(s.get_line());
		star_name->set_name("*");
		node->push_name(star_name);
	}

	if (lparen) {
		s.set_implicit_line_joining(joining);
		match(TOK_RPAREN);
	}

	node->set_level(dot_count);

	return node;
}

std::string Parser::dotted_name()
{
	std::string name = s.get_last_word();
	match(TOK_IDENT);

	while (cur_tok == TOK_DOT) {
		match(TOK_DOT);
		name += "." + s.get_last_word();
		match(TOK_IDENT);
	}

	return name;
}

AliasNode* Parser::dotted_as_name()
{
	AliasNode* node = new AliasNode(s.get_line());
	std::string name = dotted_name();
	node->set_name(name);

	if (cur_tok == TOK_AS) {
		match(TOK_AS);
		node->set_asname(s.get_last_word());
		match(TOK_IDENT);
	}

	return node;
}

ASTNode* Parser::arguments()
{
	ArgumentsNode* node = new ArgumentsNode(s.get_line());
	ASTNode* p = nullptr;
	bool have_default = false;
	ASTNode* vararg = nullptr;
	ASTNode* kwarg = nullptr;

	if (cur_tok != TOK_RPAREN) {
		p = test();
		p->set_context(EC_PARAM);
		node->push_arg(p);

		if (cur_tok == TOK_EQL) {
			match(TOK_EQL);
			have_default = true;
			ASTNode* dfl = test();
			if (!dfl) diag.error(s.get_line(), s.get_col(), "invalid syntax");
			node->push_default(dfl);
		}

		while (cur_tok == TOK_COMMA) {
            match(TOK_COMMA);

			if (cur_tok == TOK_STAR) {
				match(TOK_STAR);
				if (cur_tok != TOK_COMMA) {
					vararg = test();
				}
			} else if (cur_tok == TOK_STARSTAR) {
				match(TOK_STARSTAR);
				kwarg = test();
			} else {
				p = test();
				p->set_context(EC_PARAM);
				node->push_arg(p);

				if (cur_tok == TOK_EQL) {
					match(TOK_EQL);

					have_default = true;
					ASTNode* dfl = test();
					if (!dfl) diag.error(s.get_line(), s.get_col(), "invalid syntax");
					node->push_default(dfl);
				} else if (have_default) {
					diag.error(s.get_line(), s.get_col(), "non-default argument follows default argument");
				}
			}
		}
	}

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
