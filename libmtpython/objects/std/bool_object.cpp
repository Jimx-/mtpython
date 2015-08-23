#include <string>
#include <iostream>
#include <unordered_map>
#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "objects/std/bool_object.h"
#include "exceptions.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef bool_typedef(std::string("bool"), std::unordered_map<std::string, M_BaseObject*>{
});

M_StdBoolObject::M_StdBoolObject(bool x) : M_StdIntObject(x ? 1 : 0)
{
}

bool M_StdBoolObject::i_is(ObjSpace* space, M_BaseObject* other)
{
	M_StdBoolObject* as_bool = M_STDBOOLOBJECT(other);
	if (!as_bool) return false;

	return intval == as_bool->intval;
}

mtpython::interpreter::Typedef* M_StdBoolObject::_bool_typedef()
{
	return &bool_typedef;
}

mtpython::interpreter::Typedef* M_StdBoolObject::get_typedef()
{
	return &bool_typedef;
}
