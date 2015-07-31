#include <string>
#include <iostream>
#include "utils/source_buffer.h"
#include "parse/parser.h"

int main(int argc, char * argv[])
{
	mtpython::parse::Parser p("a.py");

	/*p.read_token();
	while (p.last_token() != mtpython::parse::TOK_EOF) {
		std::cout << mtpython::parse::tok2str(p.last_token()) << std::endl;
		p.read_token();
	}*/

	p.parse();

	return 0;
}