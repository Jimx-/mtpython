#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>

namespace mtpython {
	namespace parse {
		typedef enum {
			TOK_FILE_INPUT, TOK_NEWLINE,
			TOK_INDENT, TOK_DEDENT, TOK_IMPORT, TOK_DEF, TOK_LAMBDA, TOK_DEL, TOK_YIELD, TOK_FROM, TOK_AT, TOK_ASSERT,
			TOK_ELSE, TOK_ELIF, TOK_FOR, TOK_IF, TOK_WHILE, TOK_TRY, TOK_EXCEPT, TOK_FINALLY, TOK_RAISE, TOK_GLOBAL,
			TOK_EQL, TOK_IDENT, TOK_BREAK, TOK_CONTINUE, TOK_CLASS, TOK_IN, TOK_PASS, TOK_IS, TOK_IS_NOT, TOK_ERROR, 

			TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_STARSTAR, TOK_SLASH, TOK_SLASHSLASH, TOK_PERCENT, 
			TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE, TOK_LSQUARE, TOK_RSQUARE, TOK_CARET,
			TOK_EQLEQL, TOK_COMMA, TOK_DOT, TOK_COLON, TOK_NOT_IN,
			TOK_AND, TOK_OR, TOK_NOT, TOK_AMP, TOK_VERTBAR, TOK_TILDE, TOK_LSSLSS, TOK_GTRGTR,
			TOK_INTLITERAL, TOK_LONGLITERAL, TOK_FLOATLITERAL, TOK_DOUBLELITERAL,
			TOK_STRINGLITERAL, TOK_TRUE, TOK_FALSE, TOK_NONE, TOK_AS, TOK_ELLIPSIS, TOK_WITH,
			TOK_PLUSPLUS, TOK_MINUSMINUS, TOK_PLUSEQ, TOK_MINUSEQ, TOK_STAREQ, TOK_SLASHEQ, TOK_CARETEQ, TOK_AMPEQ,
			TOK_NEQ, TOK_LSS, TOK_GTR, TOK_LEQ, TOK_GEQ, TOK_SEMICOLON, TOK_RETURN, TOK_EOF
		} Token;

		typedef enum {
			OP_POS, OP_NEG, OP_NOT, OP_BITNOT,
			INVALID_UNOP
		} UnaryOper;

		typedef enum {
			OP_PLUS, OP_MINUS, OP_MUL, OP_DIV, OP_MOD,  /* + - * / % */
			OP_PLUSEQ, OP_MINUSEQ, OP_MULEQ, OP_DIVEQ, OP_MODEQ,	/* += -= *= /= %= */
			OP_SHL, OP_SHR, OP_AND, OP_OR, OP_BITAND, OP_BITOR,	OP_BITXOR, /* << >> && || & | ^ */
			OP_POWER,	/* ** */
			INVALID_BINOP
		} BinaryOper;

		typedef enum {
			OP_LSS, OP_GTR, OP_LSSEQ, OP_GTREQ, OP_EQ, OP_NOTEQ,		/* < > <= >= == != */
			OP_IN, OP_NOT_IN, OP_IS, OP_IS_NOT,		/* is is not */
			INVALID_CMPOP,
		} CmpOper;

		/* token -> string : debug only */
		std::string tok2str(Token tok);
		UnaryOper tok2unop(Token tok);
		BinaryOper tok2binop(Token tok);
		std::string binop2str(BinaryOper op);
		std::string unaryop2str(UnaryOper op);
		std::string cmpop2str(CmpOper op);
		CmpOper tok2cmpop(Token tok);
	}
}	

#endif
