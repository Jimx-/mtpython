#include "vm/vm.h"
#include "interpreter/compiler.h"
#include "interpreter/function.h"
#include "objects/obj_space.h"

using namespace mtpython::objects;
using namespace mtpython::vm;
using namespace mtpython::interpreter;

BaseCompiler* ObjSpace::get_compiler(ThreadContext* context)
{
	return new PyCompiler(context);
}

M_BaseObject* ObjSpace::call_args(ThreadContext* context, M_BaseObject* func, Arguments& args)
{
	Function* as_func = dynamic_cast<Function*>(func);
	if (!as_func) {
		return as_func->call_args(context, args);
	}

	return nullptr;
}

M_BaseObject* ObjSpace::call_function(ThreadContext* context, M_BaseObject* func, std::initializer_list<M_BaseObject*> args)
{
	Arguments arguments(args);

	return call_args(context, func, arguments);
}
