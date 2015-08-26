#include "interpreter/function.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

Function::Function(ObjSpace* space, Code* code, M_BaseObject* globals)
	: space(space), name(code->get_name()), code(code), func_globals(globals)
{

}

M_BaseObject* Function::call_args(mtpython::vm::ThreadContext* context, Arguments& args)
{
	return get_code()->funcrun(context, this, args);
}

M_BaseObject* Function::call_obj_args(mtpython::vm::ThreadContext* context, M_BaseObject* obj, Arguments& args)
{
	return get_code()->funcrun_obj(context, this, obj, args);
}
