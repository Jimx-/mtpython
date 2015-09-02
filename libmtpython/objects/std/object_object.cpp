#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "interpreter/error.h"
#include "objects/std/object_object.h"
#include <string>

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static Typedef object_typedef("object", {
	{ "__getattribute__", new InterpFunctionWrapper("__getattribute__", M_StdObjectObject::__getattribute__) },
	{ "__setattr__", new InterpFunctionWrapper("__setattr__", M_StdObjectObject::__setattr__) },
});

Typedef* M_StdObjectObject::get_typedef()
{
	return &object_typedef;
}

Typedef* M_StdObjectObject::_object_typedef()
{
	return &object_typedef;
}

M_BaseObject* M_StdObjectObject::__getattribute__(mtpython::vm::ThreadContext* context, M_BaseObject* obj,
												  M_BaseObject* attr)
{
	ObjSpace* space = context->get_space();
	const std::string& name = space->unwrap_str(attr);
	M_BaseObject* descr = space->lookup(obj, name);

	M_BaseObject* value = obj->get_dict_value(space, name);
	if (value) return value;

	if (descr) return space->get(descr, obj);
	throw InterpError::format(space, space->AttributeError_type(), "object has no attribute '%s'", name.c_str());
}


M_BaseObject* M_StdObjectObject::__setattr__(mtpython::vm::ThreadContext* context, M_BaseObject* obj,
											 M_BaseObject* attr, M_BaseObject* value)
{
	ObjSpace* space = context->get_space();
	const std::string& name = space->unwrap_str(attr);
	if (obj->set_dict_value(space, name, value)) return nullptr;

	throw InterpError::format(space, space->AttributeError_type(), "object has no attribute '%s'", name.c_str());
}
