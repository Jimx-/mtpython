#include "parse/scanner.h"
#include <sstream>

using namespace std;
using namespace mtpython::parse;
using namespace mtpython::utils;

Scanner::Scanner(SourceBuffer* sb, Diagnostics* diag) : buf(sb), diagnostics(diag)
{
	line = 1;
	col = 0;
	/* read the first char */
	last_char = read_char();
	indentation.push(0);
	dedentation_count = 0;
	at_start = true;
	update_indent = false;
	implicit_line_joining = false;
}

/* read_char: read a char from input stream, -1 if eof */
char Scanner::read_char()
{
	char ret = 0;
	
    if (buf->eof()) return -1;
    /* can not use input_file >> ret because it will skip white space char */
	ret = buf->read();
	col++;

	return ret;	
}

Token Scanner::look_up_res_word(const string& key)
{
	std::unordered_map<string, Token>::const_iterator got = res_words.find(key);

	return (got == res_words.end()) ? TOK_IDENT : got->second;
}

void Scanner::add_res_word(const string& key, Token tok)
{
	res_words[key] = tok;
} 

char _digit[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

int Scanner::char2digit(int base, char ch) 
{
	int i, ret = -1;

	if (ch > 0x7f) {
	    diagnostics->error(line, col, "illegal nonascii digit");
	}

	for (i = 0; i < base; i++) {
		if (i <= 9) {
			if (ch == _digit[i]) {
				ret = i;
				break;
			}
		}
		else if (ch == _digit[i] || ch == tolower(_digit[i])) {
			ret = i;
			break;
		}
	}
	
	return ret;
}

/* scan a number, radix can be 8, 10 or 16 */
/* use last_strnum to save the number,
 * and Parser will convert it later */
/* return INTLITERAL, LONGLITERAL, FLOATLITERAL or
 * DOUBLELITERAL */
Token Scanner::scan_number(int rad)
{
	radix = rad;

	while (char2digit(rad, last_char) >= 0) {
		last_strnum = last_strnum + last_char;
	    last_char = read_char();
	}
	if (rad == 16 && last_char == '.') {
		return scan_hex_fraction_and_suffix();
	} else if (rad == 16 && (last_char == 'p' || last_char == 'P')) {
	    return scan_hex_exponent_and_suffix();
	} else if (rad <= 10 && last_char == '.') {
	    last_strnum = last_strnum + last_char;
	    last_char = read_char();
	    return scan_fraction_and_suffix();
	} else if (radix <= 10 &&
			(last_char == 'e' || last_char == 'E' ||
			last_char == 'f' || last_char == 'F' ||
			last_char == 'd' || last_char == 'D')) {
		return scan_fraction_and_suffix();
	} else {
	    if (last_char == 'l' || last_char == 'L') {
			last_char = read_char();
			return TOK_LONGLITERAL;
	    } else {
			return TOK_INTLITERAL;
	    }
	}
}

void Scanner::scan_fraction()
{
	while (char2digit(10, last_char) >= 0) {
		last_strnum = last_strnum + last_char;
		last_char = read_char();
	}
	if (last_char == 'e' || last_char == 'E') {
	    last_strnum = last_strnum + last_char;
		last_char = read_char();
		if (last_char == '+' || last_char == '-') {
			last_strnum = last_strnum + last_char;
			last_char = read_char();
	    }
		if (isdigit(last_char)) {
			do {
				last_strnum = last_strnum + last_char;
				last_char = read_char();
			} while (isdigit(last_char));
			return;
	    }
	    diagnostics->error(line, col, "malformed floating point literal");
	}
}

Token Scanner::scan_fraction_and_suffix()
{
	scan_fraction();
	if (last_char == 'f' || last_char == 'F') {
		last_char = read_char();
		return TOK_FLOATLITERAL;
	} else {
		if (last_char == 'd' || last_char == 'D') {
			last_char = read_char();
		}
		return TOK_DOUBLELITERAL;
	}
}

Token Scanner::scan_hex_fraction_and_suffix()
{
	radix = 16;
	last_strnum = last_strnum + last_char;
	last_char = read_char();
	
	while (char2digit(16, last_char) >= 0) {
	    last_strnum = last_strnum + last_char;
		last_char = read_char();
	}
	return scan_hex_exponent_and_suffix();
}

Token Scanner::scan_hex_exponent_and_suffix()
{
	if (last_char == 'p' || last_char == 'P') {
		last_strnum = last_strnum + last_char;
		last_char = read_char();
		if (last_char == '+' || last_char == '-') {
			last_strnum = last_strnum + last_char;
			last_char = read_char();	    
		}
		if (isdigit(last_char)) {
			do {
				last_strnum = last_strnum + last_char;
				last_char = read_char();
			} while (isdigit(last_char));
		} else {
			diagnostics->error(line, col, "malformed floating point literal");
			return TOK_ERROR;
		}
	} else {
	    diagnostics->error(line, col, "malformed floating point literal");
		return TOK_ERROR;
	}

	if (last_char == 'f' || last_char == 'F') {
		last_char = read_char();
		return TOK_FLOATLITERAL;
	} else {
	    if (last_char == 'd' || last_char == 'D') {
			last_char = read_char();
	    }
	    return TOK_DOUBLELITERAL;
	}
}

char Scanner::scan_char_lit()
{
	if (last_char == '\\') { /* escape */
		last_char = read_char();
		switch (last_char) {
		case 'b':
			last_char = read_char();
			return '\b';
		case 't':
			last_char = read_char();
			return '\t';
		case 'n':
			last_char = read_char();
			return '\n';
		case 'r':
			last_char = read_char();
			return '\r';
		case '\'':
			last_char = read_char();
			return '\'';
		case '\"':
			last_char = read_char();
			return '\"';
		case '\\':
			last_char = read_char();
			return '\\';
		}
		char tmp;
		tmp = last_char;
		last_char = read_char();
		return tmp;
	} else {
		char tmp;
		tmp = last_char;
		last_char = read_char();
		return tmp;
	}
}


Token Scanner::get_token()
{
	if (at_start) {
		at_start = false;
		SourceType type = buf->get_src_type();

		switch (type) {
		case ST_FILE_INPUT:
			return TOK_FILE_INPUT;
		default:
			return TOK_ERROR;
		}
	}

	if (dedentation_count > 0) {
		dedentation_count--;
		return TOK_DEDENT;
	}

	bool new_line = false;

	last_word = "";
	//first skip any empty character
	if (!update_indent) {
		while ((last_char == ' ') || (last_char == '\t') ||
			(last_char == 0)) {
			last_char = read_char();
		}
	}

    /* encounted a new line */
    if ((last_char == '\n') || (last_char == '\r')) {
    	line++; 
		col = 0;
    	new_line = true;
    	last_char = read_char();
    }
	// skip all duplicated CR LF
	while ((last_char == '\n') || (last_char == '\r')) {
		last_char = read_char();
    }

	/* omit newline */
	if (implicit_line_joining && new_line) {
		return get_token();
	}

    if (new_line) {
    	update_indent = true;
    	return TOK_NEWLINE;
    }

    if (update_indent) {	/* calculate indentation */
    	update_indent = false;
    	int indentation_level = 0;
    	char indentation_char = last_char;
    	while ((last_char == ' ') || (last_char == '\t')) {
    		if (last_char != indentation_char) { /* error: mixed tabs and spaces */
    			diagnostics->error(line, col, "inconsistent use of tabs and spaces in indentation");
    			return TOK_ERROR;
    		}
    		indentation_level++;
    		last_char = read_char();
    	}

    	/* indent */
    	if (indentation_level > indentation.top()) {
    		indentation.push(indentation_level);
    		return TOK_INDENT;
    	} else if (indentation_level < indentation.top()) {
    		while (indentation.top() > indentation_level) {
    			indentation.pop();
    			dedentation_count++;
    		}
    		dedentation_count--;
    		return TOK_DEDENT;
    	}
    }

	char str_prefix = '\0';
	if (isalpha(last_char) || last_char == '_') { // reserved word or id
		if (last_char == 'r' || last_char == 'u' || last_char == 'b') {
			str_prefix = last_char;
			last_char =  read_char();
			if (last_char == '"' || last_char == '\'') goto scan_str_lit;
		}

		if (str_prefix) last_word = last_word + str_prefix;

		while (isalpha(last_char) || isdigit(last_char) || last_char == '_') {
			last_word = last_word + last_char;
			last_char = read_char();
			if (last_char == -1)
				break;
		}
		return look_up_res_word(last_word);
	}

	/* octal or hex */
	else if (last_char == '0') {
		last_strnum = "";
		last_char = read_char();
		if (last_char == '.') {	/* 0. */
			last_strnum = "0.";
			last_char = read_char();
			return scan_fraction_and_suffix();
		}
		/* 0x or 0X - hex */
		else if (last_char == 'x' || last_char == 'X') {
			last_char = read_char();
			if (char2digit(16, last_char) < 0) {
				diagnostics->error(line, col, "invalid hex number");
				return TOK_ERROR;
			} else {
				return scan_number(16);
			}
		/* 0o or 0O - oct */
		} else if (last_char == 'o' || last_char == 'O') {
			last_char = read_char();
			return scan_number(8);
		} else {
			if (!isdigit(last_char)) {
				last_strnum = "0";
				return TOK_INTLITERAL;
			}
		}
	}

	else if (isdigit(last_char) && last_char != '0') {
		last_strnum = "";
		return scan_number(10);
	}

	/*
	else if (isdigit(last_char)) { // number
		last_int = 0;
		do {
			last_int = last_int * 10 + (last_char - '0');
			last_char = read_char();
			if (last_char == -1)
				break;
		} while (isdigit(last_char));
		return TOK_NUMBER;
	}*/

	/* character literal */
	/* else if (last_char == '\'') {
		last_char = read_char();
		if (last_char == '\'') {
			last_char = read_char();
			diagnostics->error(line, col, "empty character literal");
			return TOK_ERROR;
		}
		else if (last_char == '\n' || last_char == '\r') {
			last_char = read_char();
			diagnostics->error(line, col, "illegal line end in character literal");
			return TOK_ERROR;
		}
		last_char_lit = scan_char_lit();
		if (last_char == '\'') {
			last_char = read_char();
			return TOK_CHARLITERAL;
		} else {
			last_char = read_char();
			diagnostics->error(line, col, "unclosed character literal");
			return TOK_ERROR;
		}
	} */

	/* string literal */
	else if (last_char == '\"' || last_char == '\'') {
		scan_str_lit:
		char end_char = last_char;
		bool triple = false;
		last_string = "";
		last_char = read_char();
		if (last_char == end_char) {
			last_char = read_char();
			if (last_char == end_char){
				triple = true;
				last_char = read_char();
			}
			else {
				last_string = "";
				return TOK_STRINGLITERAL;
			}
		}
		int expected_end_char = triple ? 3 : 1;
		int end_char_count = 0;
		while (true) {
			if (last_char == end_char) {
				end_char_count++;
				if (end_char_count == expected_end_char)
					break;
				else {
					last_char = read_char();
					continue;
				}
			} else
				end_char_count = 0;

			if ((last_char == '\n' || last_char == '\r') && !triple) break;
			if (last_char == '\n') line++;
			if (last_char == -1) break;

			last_string = last_string + scan_char_lit();
		}
		if (last_char == end_char) {
			last_char = read_char();
			return TOK_STRINGLITERAL;
		} else {
			last_char = read_char();
			diagnostics->error(line, col, "unclosed string literal");
			return TOK_ERROR;
		}
	}

	/* "=" - assign, "==" - equal */
	else if (last_char == '=') {
		last_char = read_char();
		if (last_char == '=') {
			last_char = read_char();
			return TOK_EQLEQL;
		}
		else 
			return TOK_EQL;
	}

	/* "<" - less than, "<=" - less than or equal, "<>" - not equal 
	 * "<<" - left shift */
	else if (last_char == '<') {
		last_char = read_char();
		if (last_char == '=') {
			last_char = read_char();
			return TOK_LEQ;
		}
		else if (last_char == '>') {
			last_char = read_char();
			return TOK_NEQ;
		}
		else if (last_char == '<') {
			last_char = read_char();
			return TOK_LSSLSS;
		}
		else
			return TOK_LSS;
	}

	/* ">" - greater than, ">=" - greater than or equal
	 * ">>" - right shift */
	else if (last_char == '>') {
		last_char = read_char();
		if (last_char == '=') {
			last_char = read_char();
			return TOK_GEQ;
		}
		else if (last_char == '>') {
			last_char = read_char();
			return TOK_GTRGTR;
		}
		else
			return TOK_GTR;
	}

	if (last_char == '#') {	// single line comment
		while (last_char != '\n' && last_char != '\r' && last_char != -1) last_char = read_char();	/* skip until '\n''\r' */
		if (last_char != -1) return get_token();
	}

	/* "/" - divide, "/*" - comment */
	else if (last_char == '/') {
		last_char = read_char();
		if (last_char == '=') {
			last_char = read_char();
			return TOK_SLASHEQ;		/* /= */
		} else if (last_char == '/') {
			last_char = read_char();
			return TOK_SLASHSLASH;
		}
		return TOK_SLASH;
	}

	/* ":" - colon */
	else if (last_char == ':') {
		last_char = read_char();
		return TOK_COLON;
	}

	else if (last_char == ',') {
		last_char = read_char();
		return TOK_COMMA;
	}

	else if (last_char == '.') {
		last_char = read_char();
		if (last_char == '.') {
			peek_begin();
			last_char = read_char();
			if (last_char == '.') {
				last_char = read_char();
				return TOK_ELLIPSIS;
			}
			peek_end();
		}
		return TOK_DOT;
	}

	else if (last_char == ';') {
		last_char = read_char();
		return TOK_SEMICOLON;
	}

	else if (last_char == '+') {
		last_char = read_char();
		if (last_char == '+') {	/* ++ */
			last_char = read_char();
			return TOK_PLUSPLUS;
		} else if (last_char == '=') {	/* += */
			last_char = read_char();
			return TOK_PLUSEQ;
		}
		return TOK_PLUS;
	} else if (last_char == '-') {	
		last_char = read_char();
		if (last_char == '-') {		/* -- */
			last_char = read_char();
			return TOK_MINUSMINUS;
		} else if (last_char == '=') {	/* -= */
			last_char = read_char();
			return TOK_MINUSEQ;
		}
		return TOK_MINUS;
	} else if (last_char == '*') {
		last_char = read_char();
		if (last_char == '=') {	/* *= */
			last_char = read_char();
			return TOK_STAREQ;
		} else if (last_char == '*') { /* ** */ 
			last_char = read_char();
			return TOK_STARSTAR;
		}
		return TOK_STAR;
	} else if (last_char == '(') {
		last_char = read_char();
		return TOK_LPAREN;
	} else if (last_char == ')') {
		last_char = read_char();
		return TOK_RPAREN;
	} else if (last_char == '[') {
		last_char = read_char();
		return TOK_LSQUARE;
	} else if (last_char == ']') {
		last_char = read_char();
		return TOK_RSQUARE;
	} else if (last_char == '{') {
		last_char = read_char();
		return TOK_LBRACE;
	} else if (last_char == '}') {
		last_char = read_char();
		return TOK_RBRACE;
	}

	else if (last_char == '&') {
		last_char = read_char();
		return TOK_AMP;
	}

	else if (last_char == '|') {
		last_char = read_char();
		return TOK_VERTBAR;
	}

	else if (last_char == '!') {
		last_char = read_char();
		if (last_char == '=') {
			last_char = read_char();
			return TOK_NEQ;
		} else return TOK_ERROR;
	}
	
	else if (last_char == '%') {
		last_char = read_char();
		return  TOK_PERCENT;
	}

	else if (last_char == '~') {
		last_char = read_char();
		return TOK_TILDE;
	}

	if (last_char == -1 || buf->eof()) {
		/* pop all indentation */
		while (indentation.size() > 1) {
			if (indentation.top() > 0) dedentation_count++;
			indentation.pop();
		}
		if (dedentation_count > 0) {
			dedentation_count--;
			return TOK_DEDENT;
		}
		return TOK_EOF;
	}
		

	stringstream ss;
	ss << (int)last_char;
	string error_msg("unknown character: \"");
	error_msg += last_char;
	error_msg += "\"(#";
	error_msg += ss.str();
	error_msg += ")";

	diagnostics->error(line, col, error_msg);
	return TOK_ERROR;
}

int Scanner::cur_pos()
{
	return buf->tell_pos();
}

void Scanner::peek_begin()
{
	peek_start_pos = buf->tell_pos();
}

Token Scanner::peek_token()
{
	return get_token();
}

void Scanner::peek_end()
{
	buf->seek(peek_start_pos);
}

void Scanner::close_input()
{
}

