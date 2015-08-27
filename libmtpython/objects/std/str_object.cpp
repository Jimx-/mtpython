#include <string>
#include <iostream>
#include <unordered_map>
#include <functional>
#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "objects/std/str_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef str_typedef("str", std::unordered_map<std::string, M_BaseObject*>{
	{ "__repr__", new InterpFunctionWrapper("__repr__", M_StdStrObject::__repr__) },
	{ "__hash__", new InterpFunctionWrapper("__hash__", M_StdStrObject::__hash__) },
	{ "__eq__", new InterpFunctionWrapper("__eq__", M_StdStrObject::__eq__) },
});

M_StdStrObject::M_StdStrObject(const std::string& s)
{
	value = s;
}

mtpython::interpreter::Typedef* M_StdStrObject::_str_typedef()
{
	return &str_typedef;
}

mtpython::interpreter::Typedef* M_StdStrObject::get_typedef()
{
	return &str_typedef;
}

M_BaseObject* M_StdStrObject::__repr__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	return self;
}

M_BaseObject* M_StdStrObject::__hash__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	M_StdStrObject* as_str = M_STDSTROBJECT(self);
	std::hash<std::string> hash_fn;
	std::size_t hash = hash_fn(as_str->value);

	return context->get_space()->wrap_int(hash);
}

M_BaseObject* M_StdStrObject::__eq__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self, mtpython::objects::M_BaseObject* other)
{
	M_StdStrObject* self_as_str = M_STDSTROBJECT(self);
	M_StdStrObject* other_as_str = M_STDSTROBJECT(other);

	if (!self_as_str || !other_as_str) return false;

	return context->get_space()->new_bool(self_as_str->value == other_as_str->value);
}

void M_StdStrObject::dbg_print()
{
	std::cout << value;
}
