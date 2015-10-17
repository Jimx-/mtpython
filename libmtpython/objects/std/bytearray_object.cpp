#include <string>
#include <iostream>
#include <unordered_map>
#include <functional>

#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "objects/std/bytearray_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef bytearray_typedef("bytearray", {
	{ "__iter__", new InterpFunctionWrapper("__iter__", M_StdByteArrayObject::__iter__) },
});

M_StdByteArrayObject::M_StdByteArrayObject()
{
}

Typedef* M_StdByteArrayObject::_bytearray_typedef()
{
	return &bytearray_typedef;
}

Typedef* M_StdByteArrayObject::get_typedef()
{
	return &bytearray_typedef;
}

M_BaseObject* M_StdByteArrayObject::__iter__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	return context->get_space()->new_seqiter(context, self);
}
