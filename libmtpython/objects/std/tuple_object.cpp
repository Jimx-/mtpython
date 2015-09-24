#include <string>
#include <unordered_map>
#include <assert.h>

#include "interpreter/typedef.h"
#include "interpreter/error.h"
#include "interpreter/gateway.h"
#include "objects/std/tuple_object.h"
#include "objects/std/iter_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef tuple_typedef("tuple", {
	{ "__repr__", new InterpFunctionWrapper("__repr__", M_StdTupleObject::__repr__) },
	{ "__iter__", new InterpFunctionWrapper("__iter__", M_StdTupleObject::__iter__) },
	{ "__len__", new InterpFunctionWrapper("__len__", M_StdTupleObject::__len__) },
	{ "__getitem__", new InterpFunctionWrapper("__getitem__", M_StdTupleObject::__getitem__) },
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
	assert(as_tuple);

	return new M_StdTupleIterObject(as_tuple->items);
}

M_BaseObject* M_StdTupleObject::__len__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	ObjSpace* space = context->get_space();
	M_StdTupleObject* as_tuple = M_STDTUPLEOBJECT(self);
	assert(as_tuple);

	return space->wrap_int(as_tuple->items.size());
}

M_BaseObject* M_StdTupleObject::__repr__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	ObjSpace* space = context->get_space();
	M_StdTupleObject* as_tuple = M_STDTUPLEOBJECT(self);
	assert(as_tuple);

	std::string str = "(";
	for (std::size_t i = 0; i < as_tuple->items.size(); i++) {
		if (i > 0) str += ", ";
		M_BaseObject* repr_item = space->repr(as_tuple->items[i]);
		str += space->unwrap_str(repr_item);
		SAFE_DELETE(repr_item);
	}
	str += ")";

	return space->wrap_str(str);
}

M_BaseObject* M_StdTupleObject::__getitem__(mtpython::vm::ThreadContext* context, M_BaseObject* obj, M_BaseObject* key)
{
	ObjSpace* space = context->get_space();
	int index;
	try {
		index = space->unwrap_int(key, false);
	} catch(...) {
		throw InterpError::format(space, space->IndexError_type(), "tuple indices must be integers, not %s", space->get_type_name(key).c_str());
	};

	M_BaseObject* item = nullptr;
	M_StdTupleObject* as_tuple = static_cast<M_StdTupleObject*>(obj);
	if (index < 0) {
		if (index < -(as_tuple->items.size())) throw InterpError(space->IndexError_type(), space->wrap_str("tuple index out of range"));
		item = as_tuple->items[as_tuple->items.size() + index];
	} else {
		if (index >= as_tuple->items.size()) throw InterpError(space->IndexError_type(), space->wrap_str("tuple index out of range"));
		item = as_tuple->items[index];
	}

	context->gc_track_object(obj);
	context->gc_track_object(key);

	return item;
}
