#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "interpreter/error.h"
#include "interpreter/descriptor.h"
#include "objects/std/type_object.h"
#include "objects/std/object_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static Typedef object_typedef("object", {
	{ "__new__", new InterpFunctionWrapper("__new__", M_StdObjectObject::__new__) },
	{ "__init__", new InterpFunctionWrapper("__init__", M_StdObjectObject::__init__) },
	{ "__str__", new InterpFunctionWrapper("__str__", M_StdObjectObject::__str__) },
	{ "__repr__", new InterpFunctionWrapper("__repr__", M_StdObjectObject::__repr__) },
	{ "__getattribute__", new InterpFunctionWrapper("__getattribute__", M_StdObjectObject::__getattribute__) },
	{ "__setattr__", new InterpFunctionWrapper("__setattr__", M_StdObjectObject::__setattr__) },
	{ "__delattr__", new InterpFunctionWrapper("__delattr__", M_StdObjectObject::__setattr__) },
	{ "__dict__", new GetSetDescriptor(M_StdObjectObject::__dict__get, M_StdObjectObject::__dict__set) },
	{ "__class__", new GetSetDescriptor(M_StdObjectObject::__class__get) },
});

Typedef* M_StdObjectObject::get_typedef()
{
	return &object_typedef;
}

Typedef* M_StdObjectObject::_object_typedef()
{
	return &object_typedef;
}

M_BaseObject* M_StdObjectObject::__new__(mtpython::vm::ThreadContext* context, const Arguments& args)
{
	static Signature new_signature({ "type" }, "args", "kwargs", {});
	ObjSpace* space = context->get_space();

	std::vector<M_BaseObject*> scope;
	args.parse("__new__", nullptr, new_signature, scope);
	M_BaseObject* wrapped_type = scope[0];

	M_StdObjectObject* instance = new M_StdObjectObject(wrapped_type);

	M_StdTypeObject* type_obj = dynamic_cast<M_StdTypeObject*>(wrapped_type);
	if (!type_obj)
		throw InterpError::format(space, space->TypeError_type(), "object.__new__(X): X is not a type object (%s)", space->get_type_name(wrapped_type).c_str());

	if (type_obj->has_dict()) {
		/* TODO: create the dict when needed */
		instance->set_dict(space->new_dict(context));
	}

	return space->wrap(context, instance);
}

M_BaseObject* M_StdObjectObject::__init__(mtpython::vm::ThreadContext* context, const Arguments& args)
{
	ObjSpace* space = context->get_space();
	return space->wrap_None();
}

M_BaseObject* M_StdObjectObject::__str__(mtpython::vm::ThreadContext* context, M_BaseObject* obj)
{
	ObjSpace* space = context->get_space();
	M_StdTypeObject* type = static_cast<M_StdTypeObject*>(space->type(obj));
	M_BaseObject* impl = type->lookup("__repr__");
	if (!impl) {
		throw InterpError(space->TypeError_type(), space->wrap_str(context, "operand does not support unary str"));
	}
	return space->get_and_call_function(context, impl, { obj });
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
	std::string name = space->unwrap_str(attr);
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

M_BaseObject* M_StdObjectObject::__dict__get(mtpython::vm::ThreadContext* context, M_BaseObject* obj)
{
	return obj->get_dict(context->get_space());
}

void M_StdObjectObject::__dict__set(mtpython::vm::ThreadContext* context, M_BaseObject* obj,
											 M_BaseObject* value)
{
	ObjSpace* space = context->get_space();
	M_StdObjectObject* as_obj = static_cast<M_StdObjectObject*>(obj);

	as_obj->lock();
	as_obj->dict = value;
	as_obj->unlock();
}

M_BaseObject* M_StdObjectObject::__class__get(mtpython::vm::ThreadContext* context, M_BaseObject* obj)
{
	return context->get_space()->type(obj);
}
