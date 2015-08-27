#include <string>
#include <unordered_map>
#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "objects/std/tuple_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef tuple_typedef("tuple", std::unordered_map<std::string, M_BaseObject*>{

});

mtpython::interpreter::Typedef* M_StdTupleObject::_tuple_typedef()
{
	return &tuple_typedef;
}

mtpython::interpreter::Typedef* M_StdTupleObject::get_typedef()
{
	return &tuple_typedef;
}
