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

	Code* get_code() { return code; }

	objects::M_BaseObject* get_globals() { return func_globals; }
	
	objects::M_BaseObject* call_args(vm::ThreadContext* context, Arguments& args);
	objects::M_BaseObject* call_obj_args(vm::ThreadContext* context, objects::M_BaseObject* obj, Arguments& args);
};

}
}

#endif /* _INTERPRETER_FUNCTION_H_ */
