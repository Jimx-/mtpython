#include "parse/token.h"

using namespace mtpython::parse;

std::string mtpython::parse::tok2str(Token tok)
{
	switch (tok) {
	case TOK_FILE_INPUT:
		return "FILE INPUT";
	case TOK_ELSE:
		return "ELSE";
	case TOK_ELIF:
		return "ELIF";
	case TOK_IF:
		return "IF";
	case TOK_FOR:
		return "FOR";
	case TOK_WHILE:
		return "WHILE";
	case TOK_EQL:
		return "=";
	case TOK_TRY:
		return "TRY";
	case TOK_EXCEPT:
		return "EXCEPT";
	case TOK_FINALLY:
		return "FINALLY";
	case TOK_BREAK:
		return "BREAK";
	case TOK_CONTINUE:
		return "CONTINUE";
	case TOK_IDENT:
		return "IDENT";
	case TOK_INDENT:
		return "INDENT";
	case TOK_DEDENT:
		return "DEDENT";
	case TOK_IN:
		return "IN";
	case TOK_INTLITERAL:
		return "INT_LITERAL";
	case TOK_LONGLITERAL:
		return "LONG_LITERAL";
	case TOK_STRINGLITERAL:
		return "STRING_LITERAL";
	case TOK_FLOATLITERAL:
		return "FLOAT_LITERAL";
	case TOK_DOUBLELITERAL:
		return "DOUBLE_LITERAL";
	case TOK_TRUE:
		return "TRUE";
	case TOK_FALSE:
		return "FALSE";
	case TOK_NONE:
		return "NONE";
	case TOK_CLASS:
		return "CLASS";
	case TOK_ERROR:
		return "ERROR";
	case TOK_PLUS:
		return "+";
	case TOK_MINUS:
		return "-";
	case TOK_STAR:
		return "*";
	case TOK_SLASH:
		return "/";
	case TOK_PERCENT:
		return "%";
	case TOK_LPAREN:
		return "(";
	case TOK_RPAREN:
		return ")";
	case TOK_LBRACE:
		return "{";
	case TOK_RBRACE:
		return "}";
	case TOK_EQLEQL:
		return "==";
	case TOK_COMMA:
		return ",";
	case TOK_DOT:
		return ".";
	case TOK_PLUSPLUS:
		return "++";
	case TOK_MINUSMINUS:
		return "--";
	case TOK_PLUSEQ:
		return "+=";
	case TOK_MINUSEQ:
		return "-=";
	case TOK_STAREQ:
		return "*=";
	case TOK_SLASHEQ:
		return "/=";
	case TOK_AND:
		return "AND";
	case TOK_OR:
		return "OR";
	case TOK_NOT:
		return "NOT";
	case TOK_AMP:
		return "&";
	case TOK_VERTBAR:
		return "|";
	case TOK_TILDE:
		return "~";
	case TOK_CARET:
		return "^";
	case TOK_NEQ:
		return "<>";
	case TOK_LSS:
		return "<";
	case TOK_GTR:
		return ">";
	case TOK_LSSLSS:
		return "<<";
	case TOK_GTRGTR:
		return ">>";
	case TOK_LEQ:
		return "<=";
	case TOK_GEQ:
		return ">=";
	case TOK_COLON:
		return ":";
	case TOK_SEMICOLON:
		return ";";
	case TOK_RETURN:
		return "RETURN";
	case TOK_EOF:
		return "EOF";
	case TOK_NEWLINE:
		return "NEWLINE";
	case TOK_IMPORT:
		return "IMPORT";
	case TOK_DEF:
		return "DEF";
	case TOK_LAMBDA:
		return "LAMBDA";
	case TOK_DEL:
		return "DEL";
	case TOK_YIELD:
		return "YIELD";
	case TOK_FROM:
		return "FROM";
	case TOK_RAISE:
		return "RAISE";
	case TOK_PASS:
		return "PASS";
	case TOK_STARSTAR:
		return "**";
	case TOK_SLASHSLASH:
		return "//";
	case TOK_LSQUARE:
		return "[";
	case TOK_RSQUARE:
		return "]";
	case TOK_IS:
		return "IS";
	case TOK_IS_NOT:
		return "IS NOT";
	case TOK_NOT_IN:
		return "NOT IN";
	case TOK_AS:
		return "AS";
	case TOK_ELLIPSIS:
		return "...";
	case TOK_WITH:
		return "WITH";
	case TOK_AT:
		return "@";
	}
	return "UNKNOWN TOKEN";
}

std::string mtpython::parse::binop2str(BinaryOper op)
{
	switch (op) {
	case OP_PLUS:
		return "+";
	case OP_MINUS:
		return "-";
	case OP_MUL:
		return "*";
	case OP_DIV:
		return "/";
	case OP_MOD:
		return "%";
	case OP_SHL:
		return "<<";
	case OP_SHR:
		return ">>";
	case OP_AND:
		return "and";
	case OP_OR:
		return "or";
	case OP_BITAND:
		return "&";
	case OP_BITOR:
		return "|";
	case OP_BITXOR:
		return "^";
	case OP_POWER:
		return "**";
	case OP_PLUSEQ:
		return "+=";
	case OP_MINUSEQ:
		return "-=";
	case OP_MULEQ:
		return "*=";
	case OP_DIVEQ:
		return "/=";
	}

	return "INVALID BINOP";
}

std::string mtpython::parse::unaryop2str(UnaryOper op)
{
	switch (op) {
	case OP_POS:
		return "+";
	case OP_NEG:
		return "-";
	case OP_NOT:
		return "!";
	case OP_BITNOT:
		return "~";
	}

	return "INVALID UNARYOP";
}

std::string mtpython::parse::cmpop2str(CmpOper op)
{
	switch (op) {
	case OP_EQ:
		return "==";
	case OP_NOTEQ:
		return "!=";
	case OP_LSS:
		return "<";
	case OP_GTR:
		return ">";
	case OP_LSSEQ:
		return "<=";
	case OP_GTREQ:
		return ">=";
	case OP_IN:
		return "in";
	}

	return "INVALID CMPOP";
}

UnaryOper mtpython::parse::tok2unop(Token tok)
{
	switch (tok) {
	case TOK_PLUS:
		return OP_POS;
	case TOK_MINUS:
		return OP_NEG;
	case TOK_NOT:
		return OP_NOT;
	case TOK_TILDE:
		return OP_BITNOT;
	}
	return INVALID_UNOP;
}

BinaryOper mtpython::parse::tok2binop(Token tok)
{
	switch (tok) {
	case TOK_PLUS:
		return OP_PLUS;
	case TOK_MINUS:
		return OP_MINUS;
	case TOK_STAR:
		return OP_MUL;
	case TOK_SLASH:
		return OP_DIV;
	case TOK_PERCENT:
		return OP_MOD;
	case TOK_PLUSEQ:
		return OP_PLUSEQ;
	case TOK_MINUSEQ:
		return OP_MINUSEQ;
	case TOK_STAREQ:
		return OP_MULEQ;
	case TOK_SLASHEQ:
		return OP_DIVEQ;
	case TOK_AND:
		return OP_AND;
	case TOK_OR:
		return OP_OR;
	case TOK_AMP:
		return OP_BITAND;
	case TOK_VERTBAR:
		return OP_BITOR;
	case TOK_CARET:
		return OP_BITXOR;
	case TOK_LSSLSS:
		return OP_SHL;
	case TOK_GTRGTR:
		return OP_SHR;
	case TOK_STARSTAR:
		return OP_POWER;
	default:
		break;
	}
	return INVALID_BINOP;
}

CmpOper mtpython::parse::tok2cmpop(Token tok)
{
	switch (tok) {
	case TOK_EQLEQL:
		return OP_EQ;
	case TOK_NEQ:
		return OP_NOTEQ;
	case TOK_LSS:
		return OP_LSS;
	case TOK_GTR:
		return OP_GTR;
	case TOK_LEQ:
		return OP_LSSEQ;
	case TOK_GEQ:
		return OP_GTREQ;
	case TOK_IN:
		return OP_IN;
	case TOK_NOT_IN:
		return OP_NOT_IN;
	case TOK_IS:
		return OP_IS;
	case TOK_IS_NOT:
		return OP_IS_NOT;
	}

	return INVALID_CMPOP;
}
