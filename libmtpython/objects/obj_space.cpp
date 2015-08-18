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

M_BaseObject* ObjSpace::execute_binop(ThreadContext* context, M_BaseObject* impl, M_BaseObject* left, M_BaseObject* right)
{
	Function* func = dynamic_cast<Function*>(impl);
	if (!func) return nullptr;

	return call_function(context, func, {left, right});
}	

#define DEF_BINARY_OPER(name, lname, rname) \
	M_BaseObject* ObjSpace::##name(ThreadContext* context, M_BaseObject* obj1, M_BaseObject* obj2) \
	{	\
		M_BaseObject* type1 = type(obj1);	\
		M_BaseObject* type2 = type(obj2);	\
		M_BaseObject* left_cls;		\
		M_BaseObject* left_impl = lookup_type_cls(type1, std::string(#lname), left_cls); 	\
		M_BaseObject* result = execute_binop(context, left_impl, obj1, obj2);	\
		return result;	\
	}


DEF_BINARY_OPER(add, __add__, __radd__)

M_BaseObject* ObjSpace::call_args(ThreadContext* context, M_BaseObject* func, Arguments& args)
{
	Function* as_func = dynamic_cast<Function*>(func);
	if (as_func) {
		return as_func->call_args(context, args);
	}

	return nullptr;
}

M_BaseObject* ObjSpace::call_function(ThreadContext* context, M_BaseObject* func, std::initializer_list<M_BaseObject*> args)
{
	Arguments arguments(args);

	return call_args(context, func, arguments);
}
