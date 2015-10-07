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
	{ "__repr__", new InterpFunctionWrapper("__repr__", M_StdSetObject::__repr__) },
	{ "__contains__", new InterpFunctionWrapper("__contains__", M_StdSetObject::__contains__) },
	{ "__le__", new InterpFunctionWrapper("__le__", M_StdSetObject::__le__) },
	{ "add", new InterpFunctionWrapper("add", M_StdSetObject::add) },
});

bool mtpython::objects::M_StdSetObject::i_issubset(M_StdSetObject * other)
{
	for (auto& item : set) {
		auto got = other->set.find(item);
		if (got == other->set.end()) return false;
	}

	return true;
}

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

M_BaseObject* M_StdSetObject::__repr__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	ObjSpace* space = context->get_space();
	M_StdSetObject* as_set = static_cast<M_StdSetObject*>(self);

	std::string str = "{";
	int i = 0;
	as_set->lock();
	for (auto& item : as_set->set) {
		if (i > 0) str += ", ";
		M_BaseObject* repr_item = space->repr(item);
		str += space->unwrap_str(repr_item);
		context->delete_local_ref(repr_item);
		i++;
	}
	as_set->unlock();
	str += "}";

	return space->wrap_str(str);
}

M_BaseObject* M_StdSetObject::__contains__(mtpython::vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* obj)
{
	ObjSpace* space = context->get_space();
	M_StdSetObject* as_set = static_cast<M_StdSetObject*>(self);

	as_set->lock();
	auto got = as_set->set.find(obj);
	bool result = got != as_set->set.end();
	as_set->unlock();

	return space->new_bool(result);
}

M_BaseObject* M_StdSetObject::__le__(mtpython::vm::ThreadContext * context, M_BaseObject * self, M_BaseObject * other)
{
	ObjSpace* space = context->get_space();

	M_StdSetObject* other_as_set = dynamic_cast<M_StdSetObject*>(other);
	if (!other_as_set) return nullptr;
	M_StdSetObject* self_as_set = static_cast<M_StdSetObject*>(self);

	if (self_as_set->set.size() > other_as_set->set.size()) return space->wrap_False();
	return space->new_bool(self_as_set->i_issubset(other_as_set));
}

Typedef* M_StdSetObject::_set_typedef()
{
	return &set_typedef;
}

Typedef* M_StdSetObject::get_typedef()
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
