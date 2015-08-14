#include <string.h>
#include <iostream>
#include <unordered_map>
#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "objects/std/int_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef int_typedef(std::string("int"), std::unordered_map<std::string, M_BaseObject*>{
	{ "__repr__", new InterpFunctionWrapper(M_StdIntObject::__repr__) },
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
	return nullptr;
}

void M_StdIntObject::dbg_print()
{
	std::cout << intval;
}
