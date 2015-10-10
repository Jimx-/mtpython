#ifndef _INTERPRETER_PYCODE_H_
#define _INTERPRETER_PYCODE_H_

#include "objects/obj_space.h"
#include "interpreter/code.h"
#include "interpreter/signature.h"
#include "vm/vm.h"
#include <vector>

namespace mtpython {
namespace interpreter {

class PyCode : public Code {
private:
	mtpython::objects::ObjSpace* space;
	int co_argcount;
	int co_kwonlyargcount;
	int co_nlocals;
	int co_stacksize;
	int co_flags;
	std::vector<unsigned char> co_code;
	std::vector<mtpython::objects::M_BaseObject*> co_consts;
	std::vector<mtpython::objects::M_BaseObject*> co_names;
	std::vector<std::string> co_varnames;
	std::vector<std::string> co_freevars;
	std::vector<std::string> co_cellvars;
	std::string co_filename;
	int co_firstlineno;
	std::vector<unsigned char> co_lnotab;

	Signature signature;
	void generate_signature();
public:
	PyCode(mtpython::objects::ObjSpace* space, int argcount, int kwonlyargcount, int nlocals, int stacksize, int flags,
			std::vector<unsigned char>& code, std::vector<mtpython::objects::M_BaseObject*>& consts, 
			std::vector<std::string>& names, std::vector<std::string>& varnames, std::vector<std::string>& freevars,
			std::vector<std::string>& cellvars, std::string& filename, std::string& name, int firstlineno, std::vector<unsigned char>& lnotab);

	std::vector<unsigned char>& get_code() { return co_code; }
	std::vector<mtpython::objects::M_BaseObject*>& get_consts() { return co_consts; }
	std::vector<mtpython::objects::M_BaseObject*>& get_names() { return co_names; }
	std::vector<std::string>& get_varnames() { return co_varnames; }
	int get_nlocals() { return co_nlocals; }
	int get_nfreevars() { return co_freevars.size(); }
	
	objects::M_BaseObject* exec_code(vm::ThreadContext* context, objects::M_BaseObject* globals, objects::M_BaseObject* locals);

	mtpython::objects::M_BaseObject* funcrun(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, Arguments& args)
	{
		return funcrun_obj(context, func, nullptr, args);
	}

	mtpython::objects::M_BaseObject* funcrun_obj(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, mtpython::objects::M_BaseObject* obj, Arguments& args);
};

}
}

#endif /* _INTERPRETER_PYCODE_H_ */
