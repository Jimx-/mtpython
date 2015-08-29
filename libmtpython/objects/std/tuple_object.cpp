#include <string>
#include <unordered_map>
#include "interpreter/typedef.h"
#include "interpreter/error.h"
#include "interpreter/gateway.h"
#include "objects/std/tuple_object.h"
#include "objects/std/iter_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef tuple_typedef("tuple", {
	{ "__iter__", new InterpFunctionWrapper("__iter__", M_StdTupleObject::__iter__) },
});

mtpython::interpreter::Typedef* M_StdTupleObject::_tuple_typedef()
{
	return &tuple_typedef;
}

mtpython::interpreter::Typedef* M_StdTupleObject::get_typedef()
{
	return &tuple_typedef;
}

M_BaseObject* M_StdTupleObject::__iter__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	ObjSpace* space = context->get_space();
	M_StdTupleObject* as_tuple = M_STDTUPLEOBJECT(self);
	if (!as_tuple) throw InterpError(space->TypeError_type(), space->wrap_str("object is not tuple"));

	return new M_StdTupleIterObject(as_tuple->items);
}
