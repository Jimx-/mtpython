#include <string.h>
#include <iostream>
#include <unordered_map>
#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "objects/std/str_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef str_typedef(std::string("str"), std::unordered_map<std::string, M_BaseObject*>{
	{ "__repr__", new InterpFunctionWrapper(M_StdStrObject::__repr__) },
});

M_StdStrObject::M_StdStrObject(std::string& s)
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

void M_StdStrObject::dbg_print()
{
	std::cout << value;
}
