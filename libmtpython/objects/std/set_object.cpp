#include <string>
#include <unordered_set>

#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "interpreter/error.h"
#include "objects/std/set_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef set_typedef("set", {
	{ "__new__", new InterpFunctionWrapper("__new__", M_StdSetObject::__new__) },
	{ "__init__", new InterpFunctionWrapper("__init__", M_StdSetObject::__init__) },
	/*{ "__repr__", new InterpFunctionWrapper("__repr__", M_StdSetObject::__repr__) },
	{ "__contains__", new InterpFunctionWrapper("__contains__", M_StdSetObject::__contains__) },*/
	{ "add", new InterpFunctionWrapper("add", M_StdSetObject::add) },
});

M_BaseObject* M_StdSetObject::__new__(mtpython::vm::ThreadContext* context, const Arguments& args)
{
	ObjSpace* space = context->get_space();
	M_BaseObject* instance = new M_StdSetObject(space);
	return space->wrap(instance);
}

M_BaseObject* M_StdSetObject::__init__(mtpython::vm::ThreadContext* context, const Arguments& args)
{
	ObjSpace* space = context->get_space();
	return space->wrap_None();
}

mtpython::interpreter::Typedef* M_StdSetObject::_set_typedef()
{
	return &set_typedef;
}

mtpython::interpreter::Typedef* M_StdSetObject::get_typedef()
{
	return &set_typedef;
}

M_BaseObject* M_StdSetObject::add(mtpython::vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* item)
{
	M_StdSetObject* as_set = static_cast<M_StdSetObject*>(self);

	as_set->lock();
	as_set->set.insert(item);
	as_set->unlock();

	return context->get_space()->wrap_None();
}
