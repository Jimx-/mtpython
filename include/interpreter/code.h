#ifndef _INTERPRETER_CODE_H_
#define _INTERPRETER_CODE_H_

#include "objects/base_object.h"
#include "interpreter/arguments.h"
#include "vm/vm.h"
#include <string>

namespace mtpython {
namespace interpreter {

/* Wrapper for compiled source code */
class Code : public mtpython::objects::M_BaseObject {
protected:
	std::string co_name;
public:
	Code(std::string& name) : co_name(name) { }

	std::string& get_name() { return co_name; }

	virtual mtpython::objects::M_BaseObject* funcrun(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, Arguments&  args)
	{
		return nullptr;
	}

	virtual mtpython::objects::M_BaseObject* funcrun_obj(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, mtpython::objects::M_BaseObject* obj, Arguments& args)
	{
		return nullptr;
	}

	virtual objects::M_BaseObject* exec_code(vm::ThreadContext* context, objects::M_BaseObject* globals, objects::M_BaseObject* locals)
	{
		return nullptr;
	}
};

}
}

#endif /* _INTERPRETER_CODE_H_ */
