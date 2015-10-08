#include "interpreter/pycode.h"
#include "interpreter/pyframe.h"
#include "interpreter/function.h"
#include "interpreter/error.h"

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

	generate_signature();
}

void PyCode::generate_signature()
{
	int argcount = co_argcount;

	std::vector<std::string> argnames(co_varnames.begin(), co_varnames.begin() + argcount);
	std::string varargname;
	std::string kwargname;

	varargname = "";
	kwargname = "";

	std::vector<std::string> kwonlyargnames;

	signature = Signature(argnames, varargname, kwargname, kwonlyargnames);
}

M_BaseObject* PyCode::exec_code(ThreadContext* context, M_BaseObject* globals, M_BaseObject* locals)
{
	ObjSpace* space = context->get_space();
	PyFrame* frame = space->create_frame(context, this, globals);
	if (!frame) return nullptr;
	
	return frame->exec();
}

M_BaseObject* PyCode::funcrun_obj(ThreadContext* context, M_BaseObject* func, M_BaseObject* obj, Arguments& args)
{
	ObjSpace* space = context->get_space();

	Function* as_func = dynamic_cast<Function*>(func);
	if (!as_func)
		throw InterpError(space->TypeError_type(), space->wrap_str(context, "expected Functon object"));
	PyFrame* frame = space->create_frame(context, this, as_func->get_globals());
	if (!frame) return nullptr;

	args.parse(as_func->get_name(), obj, signature, frame->get_local_vars(), as_func->get_defaults());

	return frame->exec();
}
