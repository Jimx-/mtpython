#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "interpreter/error.h"
#include "objects/std/type_object.h"
#include "objects/std/object_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static Typedef object_typedef("object", {
	{ "__str__", new InterpFunctionWrapper("__str__", M_StdObjectObject::__str__) },
	{ "__repr__", new InterpFunctionWrapper("__repr__", M_StdObjectObject::__repr__) },
	{ "__getattribute__", new InterpFunctionWrapper("__getattribute__", M_StdObjectObject::__getattribute__) },
	{ "__setattr__", new InterpFunctionWrapper("__setattr__", M_StdObjectObject::__setattr__) },
	{ "__delattr__", new InterpFunctionWrapper("__delattr__", M_StdObjectObject::__setattr__) },
});

Typedef* M_StdObjectObject::get_typedef()
{
	return &object_typedef;
}

Typedef* M_StdObjectObject::_object_typedef()
{
	return &object_typedef;
}

M_BaseObject* M_StdObjectObject::__str__(mtpython::vm::ThreadContext* context, M_BaseObject* obj)
{
	ObjSpace* space = context->get_space();
	M_StdTypeObject* type = static_cast<M_StdTypeObject*>(space->type(obj));
	M_BaseObject* impl = type->lookup("__repr__");
	if (!impl) {
		throw InterpError(space->TypeError_type(), space->wrap_str("operand does not support unary str"));
	}
	return space->get_and_call_function(context, impl, {obj});
}

M_BaseObject* M_StdObjectObject::__repr__(mtpython::vm::ThreadContext* context, M_BaseObject* obj)
{
	ObjSpace* space = context->get_space();
	std::string classname = space->get_type_name(obj);
	classname += " object";

	return obj->get_repr(space, classname);
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
	throw InterpError::format(space, space->AttributeError_type(), "'%s' object has no attribute '%s'", space->get_type_name(obj).c_str(), name.c_str());
}


M_BaseObject* M_StdObjectObject::__setattr__(mtpython::vm::ThreadContext* context, M_BaseObject* obj,
											 M_BaseObject* attr, M_BaseObject* value)
{
	ObjSpace* space = context->get_space();
	const std::string& name = space->unwrap_str(attr);

	M_BaseObject* descr = space->lookup(obj, name);
	if (descr) {
		if (space->lookup(descr, "__set__")) {
			space->set(descr, obj, value);
			return nullptr;
		}
	}

	if (obj->set_dict_value(space, name, value)) return nullptr;

	throw InterpError::format(space, space->AttributeError_type(), "'%s' object has no attribute '%s'", space->get_type_name(obj).c_str(), name.c_str());
}

M_BaseObject* M_StdObjectObject::__delattr__(mtpython::vm::ThreadContext* context, M_BaseObject* obj,
											 M_BaseObject* attr)
{
	ObjSpace* space = context->get_space();
	const std::string& name = space->unwrap_str(attr);
	if (obj->del_dict_value(space, name)) return nullptr;

	throw InterpError::format(space, space->AttributeError_type(), "'%s' object has no attribute '%s'", space->get_type_name(obj).c_str(), name.c_str());
}

