#include <string>
#include <assert.h>

#include <unordered_map>
#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "objects/std/bool_object.h"
#include "exceptions.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef bool_typedef("bool", {
	{ "__bool__", new InterpFunctionWrapper("__bool__", M_StdBoolObject::__bool__) },
	{ "__repr__", new InterpFunctionWrapper("__repr__", M_StdBoolObject::__repr__) },
	{ "__str__", new InterpFunctionWrapper("__str__", M_StdBoolObject::__str__) },
	{ "__and__", new InterpFunctionWrapper("__and__", M_StdBoolObject::__and__) },
});

M_StdBoolObject::M_StdBoolObject(bool x) : M_StdIntObject(x ? 1 : 0)
{
}

M_BaseObject* M_StdBoolObject::__repr__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self)
{
	M_StdBoolObject* as_bool = M_STDBOOLOBJECT(self);
	assert(as_bool);

	return context->get_space()->wrap_str(context, (as_bool->intval == 1) ? "True" : "False");
}

M_BaseObject* M_StdBoolObject::__str__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self)
{
	M_StdBoolObject* as_bool = M_STDBOOLOBJECT(self);
	assert(as_bool);

	return context->get_space()->wrap_str(context, (as_bool->intval == 1) ? "True" : "False");
}

bool M_StdBoolObject::i_is(ObjSpace* space, M_BaseObject* other)
{
	M_StdBoolObject* as_bool = M_STDBOOLOBJECT(other);
	assert(as_bool);

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

M_BaseObject* M_StdBoolObject::__bool__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	return self;
}

M_BaseObject* M_StdBoolObject::__and__(mtpython::vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* other)
{
	ObjSpace* space = context->get_space();

	M_StdBoolObject* self_as_bool = static_cast<M_StdBoolObject*>(self);
	M_StdBoolObject* other_as_bool = dynamic_cast<M_StdBoolObject*>(other);
	if (!other_as_bool) {
		return M_StdIntObject::__and__(context, self, other);
	}

	bool x = self_as_bool->intval != 0;
	bool y = other_as_bool->intval != 0;

	bool z = x && y;

	return space->new_bool(z);
}
