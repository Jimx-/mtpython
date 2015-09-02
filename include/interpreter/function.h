#ifndef _INTERPRETER_FUNCTION_H_
#define _INTERPRETER_FUNCTION_H_

#include "objects/base_object.h"
#include "objects/obj_space.h"
#include "interpreter/code.h"
#include "interpreter/arguments.h"
#include "vm/vm.h"
#include <string>

namespace mtpython {
namespace interpreter {

class Function : public objects::M_BaseObject {
private:
	objects::ObjSpace* space;
	std::string name;
	Code* code;
	objects::M_BaseObject* func_globals;
public:
	Function(objects::ObjSpace* space, Code* code, objects::M_BaseObject* globals=nullptr);
	virtual Typedef* get_typedef();

	Code* get_code() { return code; }

	objects::M_BaseObject* get_globals() { return func_globals; }
	std::string& get_name() { return name; }

	objects::M_BaseObject* call_args(vm::ThreadContext* context, Arguments& args);
	objects::M_BaseObject* call_obj_args(vm::ThreadContext* context, objects::M_BaseObject* obj, Arguments& args);

	static objects::M_BaseObject* __get__(vm::ThreadContext* context, objects::M_BaseObject* self, objects::M_BaseObject* obj, objects::M_BaseObject* type);
};

/* A method is a function bound to an instance */
class Method : public objects::M_BaseObject {
private:
	objects::ObjSpace* space;
	Function* func;
	objects::M_BaseObject* instance;
public:
	Method(objects::ObjSpace* space, Function* func, objects::M_BaseObject* instance) : space(space), func(func), instance(instance) { }

	objects::M_BaseObject* call_args(vm::ThreadContext* context, Arguments& args);
};

}
}

#endif /* _INTERPRETER_FUNCTION_H_ */
