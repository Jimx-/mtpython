#include "interpreter/pycode.h"
#include "interpreter/pyframe.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;
using namespace mtpython::vm;

PyCode::PyCode(mtpython::objects::ObjSpace* space, int argcount, int kwonlyargcount, int nlocals, int stacksize, int flags,
			std::vector<unsigned char>& code, std::vector<mtpython::objects::M_BaseObject*>& consts, 
			std::vector<std::string>& names, std::vector<std::string>& varnames, std::vector<std::string>& freevars,
			std::vector<std::string>& cellvars, std::string& filename, std::string& name, int firstlineno, std::vector<unsigned char>& lnotab) :
			Code(name),
			co_argcount(argcount), co_kwonlyargcount(kwonlyargcount), co_nlocals(nlocals), co_stacksize(stacksize), co_flags(flags),
			co_code(code), co_consts(consts), co_varnames(varnames), co_freevars(freevars), co_cellvars(cellvars),
			co_filename(filename), co_firstlineno(firstlineno), co_lnotab(lnotab)
{
	this->space = space;

	for (auto& _name : names) {
		co_names.push_back(space->new_interned_str(_name));
	}
}

M_BaseObject* PyCode::exec_code(ThreadContext* context, M_BaseObject* globals, M_BaseObject* locals)
{
	ObjSpace* space = context->get_space();
	PyFrame* frame = space->create_frame(context, this, globals);
	if (!frame) return nullptr;
	
	return frame->exec();
}
