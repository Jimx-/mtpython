#include "interpreter/pycode.h"

using namespace mtpython::interpreter;

PyCode::PyCode(mtpython::objects::ObjSpace* space, int argcount, int kwonlyargcount, int nlocals, int stacksize, int flags,
			std::vector<char>& code, std::vector<mtpython::objects::M_BaseObject*>& consts, 
			std::vector<std::string>& names, std::vector<std::string>& varnames, std::vector<std::string>& freevars,
			std::vector<std::string>& cellvars, std::string& filename, std::string& name, int firstlineno, std::vector<char>& lnotab) :
			M_Code(name),
			co_argcount(argcount), co_kwonlyargcount(kwonlyargcount), co_nlocals(nlocals), co_stacksize(stacksize), co_flags(flags),
			co_code(code), co_consts(consts), co_names(names), co_varnames(varnames), co_freevars(freevars), co_cellvars(cellvars),
			co_filename(filename), co_firstlineno(firstlineno), co_lnotab(lnotab)
{
	this->space = space;
}
