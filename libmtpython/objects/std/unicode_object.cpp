#include <string>
#include <iostream>
#include <unordered_map>
#include <functional>
#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "objects/std/unicode_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef str_typedef("str", {
	{ "__repr__", new InterpFunctionWrapper("__repr__", M_StdUnicodeObject::__repr__) },
	{ "__hash__", new InterpFunctionWrapper("__hash__", M_StdUnicodeObject::__hash__) },
	{ "__eq__", new InterpFunctionWrapper("__eq__", M_StdUnicodeObject::__eq__) },
});

M_StdUnicodeObject::M_StdUnicodeObject(const std::string& s)
{
	value = s;
}

mtpython::interpreter::Typedef* M_StdUnicodeObject::_str_typedef()
{
	return &str_typedef;
}

mtpython::interpreter::Typedef* M_StdUnicodeObject::get_typedef()
{
	return &str_typedef;
}

M_BaseObject* M_StdUnicodeObject::__repr__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	return self;
}

M_BaseObject* M_StdUnicodeObject::__hash__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	M_StdUnicodeObject* as_str = M_STDUNICODEOBJECT(self);
	std::hash<std::string> hash_fn;
	std::size_t hash = hash_fn(as_str->value);

	return context->get_space()->wrap_int(hash);
}

M_BaseObject* M_StdUnicodeObject::__eq__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self, mtpython::objects::M_BaseObject* other)
{
	M_StdUnicodeObject* self_as_str = M_STDUNICODEOBJECT(self);
	M_StdUnicodeObject* other_as_str = M_STDUNICODEOBJECT(other);

	if (!self_as_str || !other_as_str) return context->get_space()->new_bool(false);

	return context->get_space()->new_bool(self_as_str->value == other_as_str->value);
}

void M_StdUnicodeObject::dbg_print()
{
	std::cout << value;
}