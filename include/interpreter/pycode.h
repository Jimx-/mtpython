#ifndef _INTERPRETER_PYCODE_H_
#define _INTERPRETER_PYCODE_H_

#include "objects/obj_space.h"
#include "interpreter/code.h"
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
	std::vector<char> co_code;
	std::vector<mtpython::objects::M_BaseObject*> co_consts;
	std::vector<std::string> co_names;
	std::vector<std::string> co_varnames;
	std::vector<std::string> co_freevars;
	std::vector<std::string> co_cellvars;
	std::string co_filename;
	int co_firstlineno;
	std::vector<char> co_lnotab;
public:
	PyCode(mtpython::objects::ObjSpace* space, int argcount, int kwonlyargcount, int nlocals, int stacksize, int flags,
			std::vector<char>& code, std::vector<mtpython::objects::M_BaseObject*>& consts, 
			std::vector<std::string>& names, std::vector<std::string>& varnames, std::vector<std::string>& freevars,
			std::vector<std::string>& cellvars, std::string& filename, std::string& name, int firstlineno, std::vector<char>& lnotab);

	std::vector<char>& get_code() { return co_code; }
	std::vector<mtpython::objects::M_BaseObject*>& get_consts() { return co_consts; }

	virtual objects::M_BaseObject* exec_code(vm::ThreadContext* context, objects::M_BaseObject* globals, objects::M_BaseObject* locals);
};

}
}

#endif /* _INTERPRETER_PYCODE_H_ */
