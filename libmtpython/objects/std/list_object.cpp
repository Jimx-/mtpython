#include <string>
#include <unordered_map>
#include <assert.h>

#include "interpreter/typedef.h"
#include "interpreter/error.h"
#include "interpreter/gateway.h"
#include "objects/std/list_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef list_typedef("list", {
	{ "__repr__", new InterpFunctionWrapper("__repr__", M_StdListObject::__repr__) },
	{ "__len__", new InterpFunctionWrapper("__len__", M_StdListObject::__len__) },

	{ "append", new InterpFunctionWrapper("append", M_StdListObject::append) },
	{ "extend", new InterpFunctionWrapper("extend", M_StdListObject::append) },
});

mtpython::interpreter::Typedef* M_StdListObject::_list_typedef()
{
	return &list_typedef;
}

mtpython::interpreter::Typedef* M_StdListObject::get_typedef()
{
	return &list_typedef;
}

M_BaseObject* M_StdListObject::__len__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	ObjSpace* space = context->get_space();
	M_StdListObject* as_list = M_STDLISTOBJECT(self);
	assert(as_list);

	return space->wrap_int(as_list->items.size());
}

M_BaseObject* M_StdListObject::__repr__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	ObjSpace* space = context->get_space();
	M_StdListObject* as_list = M_STDLISTOBJECT(self);
	assert(as_list);

	std::string str = "[";
	for (std::size_t i = 0; i < as_list->items.size(); i++) {
		if (i > 0) str += ", ";
		M_BaseObject* repr_item = space->repr(as_list->items[i]);
		str += space->unwrap_str(repr_item);
		context->delete_local_ref(repr_item);
	}
	str += "]";

	return space->wrap_str(str);
}

M_BaseObject* M_StdListObject::append(mtpython::vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* item)
{
	M_StdListObject* as_list = M_STDLISTOBJECT(self);

	as_list->lock();
	as_list->items.push_back(item);
	as_list->unlock();

	return nullptr;
}

M_BaseObject* M_StdListObject::extend(mtpython::vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* iterable)
{
	ObjSpace* space = context->get_space();
	M_BaseObject* iter = space->iter(iterable);
	M_StdListObject* as_list = M_STDLISTOBJECT(self);

	as_list->lock();
	while (true) {
		try {
			as_list->items.push_back(space->next(iter));
		} catch (InterpError& e) {
			if (!e.match(space, space->StopIteration_type())) throw e;
			break;
		}
	}
	as_list->unlock();

	return nullptr;
}
