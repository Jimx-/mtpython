#ifndef _SCANNER_H_
#define _SCANNER_H_

#include <fstream>
#include <unordered_map>
#include <stack>
#include "parse/token.h"
#include "parse/diagnostics.h"
#include "utils/source_buffer.h"

namespace mtpython {
	namespace parse {

		// Scanner: Used for lexical analysis.
		class Scanner {
		private:
			/* current source file */
			utils::SourceBuffer buf;
			/* diagnostics */
			Diagnostics diagnostics;
			/* reserve word list */
			std::unordered_map<std::string, Token> res_words;

			std::stack<int> indentation;
			/* if dedentation_count > 0, throw a DEDENT token and dedentation_count-- */
			int dedentation_count;

			int peek_start_pos;
			bool at_start;

			/* current line and column */
			int line;
			int col;

			int radix;
			/* last word, number and char in source file */
			std::string last_word;
			char last_char;
			char last_char_lit;
			std::string last_string;
			std::string last_strnum;

			/* read a char from source file */
			char read_char();
			Token look_up_res_word(const std::string& key);
			int char2digit(int base, char ch); 
			Token scan_number(int rad);
			void scan_fraction();
			Token scan_fraction_and_suffix();
			Token scan_hex_fraction_and_suffix();
			Token scan_hex_exponent_and_suffix();
			char scan_char_lit();
		public:
			Scanner(const utils::SourceBuffer& sb, const Diagnostics& diag);

			void add_res_word(const std::string& key, Token tok);
			Token get_token();
			int cur_pos();
			void peek_begin();
			Token peek_token();
			void peek_end();
			void close_input();

			int get_line() { return line; }
			int get_col() { return col; }
			int get_radix() { return radix; }
			char get_last_char() { return last_char; }
			std::string get_last_word() { return last_word; }
			std::string get_last_string() { return last_string; }
			char get_last_char_lit() { return last_char_lit; }
			std::string get_last_strnum() { return last_strnum; }
		};
	}
}

#endif


