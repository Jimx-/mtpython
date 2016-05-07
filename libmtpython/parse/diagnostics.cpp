#include "parse/diagnostics.h"
#include "interpreter/error.h"
#include "exceptions.h"
#include <iostream>

using mtpython::parse::Diagnostics;

void Diagnostics::warning(int line, int col, const std::string& warning_msg)
{
	std::cerr << info->get_filename() << ":" << line << ":" << col << ":";
	std::cerr << "warning: " << warning_msg << std::endl;
	std::cerr << "  " << source->get_line(line) << std::endl;
	std::cerr << "  " << std::string(" ", col - 1) << "^" << std::endl; 
}

void Diagnostics::error(int line, int col, const std::string& error_msg)
{
	std::cerr << info->get_filename() << ":" << line << ":" << col << ":";
	std::cerr << "error: " << error_msg << std::endl;
	std::cerr << "  " << source->get_line(line) << std::endl;
	std::cerr << "  " << std::string(col - 1, ' ') << "^" << std::endl;

	throw interpreter::InterpError(space->SyntaxError_type(), space->wrap_str(space->current_thread(), error_msg));
}

