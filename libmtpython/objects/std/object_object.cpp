#include "interpreter/typedef.h"
#include "objects/std/object_object.h"
#include <string>

using namespace mtpython::objects;

static mtpython::interpreter::Typedef object_typedef("object", std::unordered_map<std::string, M_BaseObject*>({}));

mtpython::interpreter::Typedef* M_StdObjectObject::get_typedef()
{
	return &object_typedef;
}

mtpython::interpreter::Typedef* M_StdObjectObject::_object_typedef()
{
	return &object_typedef;
}
