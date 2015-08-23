#include <string>
#include <iostream>
#include <unordered_map>
#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "objects/std/int_object.h"
#include "exceptions.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef int_typedef(std::string("int"), std::unordered_map<std::string, M_BaseObject*>{
	{ "__repr__", new InterpFunctionWrapper(M_StdIntObject::__repr__) },
	{ "__add__", new InterpFunctionWrapper(M_StdIntObject::__add__) },
});

M_StdIntObject::M_StdIntObject(int x)
{
	intval = x;
}

M_StdIntObject::M_StdIntObject(std::string& x)
{
	intval = std::stoi(x, nullptr, 0);
}

mtpython::interpreter::Typedef* M_StdIntObject::_int_typedef()
{
	return &int_typedef;
}

mtpython::interpreter::Typedef* M_StdIntObject::get_typedef()
{
	return &int_typedef;
}

M_BaseObject* M_StdIntObject::__repr__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	ObjSpace* space = context->get_space();

	int i = space->unwrap_int(self);

	return space->wrap_str(std::to_string(i)); 
}

M_BaseObject* M_StdIntObject::__add__(mtpython::vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* other)
{
	M_StdIntObject* self_as_int = dynamic_cast<M_StdIntObject*>(self);
	if (!self_as_int) throw TypeError("Object is not int");
	M_StdIntObject* other_as_int = dynamic_cast<M_StdIntObject*>(other);
	if (!other_as_int) throw TypeError("Object is not int");

	int x = self_as_int->intval;
	int y = other_as_int->intval;

	int z = x + y;

	return context->get_space()->wrap_int(z);
}

void M_StdIntObject::dbg_print()
{
	std::cout << intval;
}
