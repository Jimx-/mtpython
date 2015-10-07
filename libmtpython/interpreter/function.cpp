#include "interpreter/function.h"
#include "interpreter/gateway.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static Typedef Function_typedef("function", {
	{ "__get__", new InterpFunctionWrapper("__get__", Function::__get__) },
});

Function::Function(ObjSpace* space, Code* code, M_BaseObject* globals)
	: space(space), name(code->get_name()), code(code), func_globals(globals)
{

}

Typedef* Function::get_typedef()
{
	return &Function_typedef;
}

M_BaseObject* Function::call_args(mtpython::vm::ThreadContext* context, Arguments& args)
{
	return get_code()->funcrun(context, this, args);
}

M_BaseObject* Function::call_obj_args(mtpython::vm::ThreadContext* context, M_BaseObject* obj, Arguments& args)
{
	return get_code()->funcrun_obj(context, this, obj, args);
}

M_BaseObject* Function::__get__(mtpython::vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* obj,
								M_BaseObject* type)
{
	ObjSpace* space = context->get_space();
	if (!obj) return self;

	Function* as_func = dynamic_cast<Function*>(self);
	return space->wrap(context, new Method(space, as_func, obj));
}

M_BaseObject* Method::call_args(mtpython::vm::ThreadContext* context, Arguments& args)
{
	return space->call_obj_args(context, func, instance, args);
}
