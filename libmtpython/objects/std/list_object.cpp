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
	{ "__contains__", new InterpFunctionWrapper("__contains__", M_StdListObject::__contains__) },
	{ "__iter__", new InterpFunctionWrapper("__iter__", M_StdListObject::__iter__) },
	{ "__getitem__", new InterpFunctionWrapper("__getitem__", M_StdListObject::__getitem__) },

	{ "append", new InterpFunctionWrapper("append", M_StdListObject::append) },
	{ "extend", new InterpFunctionWrapper("extend", M_StdListObject::extend) },
	{ "pop", new InterpFunctionWrapper("pop", M_StdListObject::pop) },
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

	return space->wrap_int(context, as_list->items.size());
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
	}
	str += "]";

	return space->wrap_str(context, str);
}

M_BaseObject* M_StdListObject::__contains__(mtpython::vm::ThreadContext* context, M_BaseObject* self,
											 M_BaseObject* obj)
{
	ObjSpace* space = context->get_space();
	M_StdListObject* as_list = static_cast<M_StdListObject*>(self);
	for (auto item : as_list->items) {
		if (space->i_eq(item, obj)) {
			return space->wrap_True();
		}
	}

	return space->wrap_False();
}

M_BaseObject* M_StdListObject::__iter__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	return context->get_space()->new_seqiter(context, self);
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

M_BaseObject* M_StdListObject::__getitem__(mtpython::vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* index)
{
	ObjSpace* space = context->get_space();
	ScopedObjectLock lock(self);

	M_StdListObject* as_list = static_cast<M_StdListObject*>(self);

	int i = space->i_get_index(index, space->IndexError_type(), space->wrap_str(context, "list index"));
	M_BaseObject* item = nullptr;
	if (i < 0) {
		if (i < -(int)(as_list->items.size())) throw InterpError(space->IndexError_type(), space->wrap_str(context, "list index out of range"));
		item = as_list->items[as_list->items.size() + i];
	} else {
		if (i >= (int)as_list->items.size()) throw InterpError(space->IndexError_type(), space->wrap_str(context, "list index out of range"));
		item = as_list->items[i];
	}

	return item;
}

M_BaseObject* M_StdListObject::pop(mtpython::vm::ThreadContext* context, const Arguments& args)
{
	static Signature pop_signature({"self", "index"});

	ObjSpace* space = context->get_space();
	std::vector<M_BaseObject*> scope;
	args.parse("pop", nullptr, pop_signature, scope, { space->wrap_int(context, -1) });

	M_BaseObject* self = scope[0];
	M_BaseObject* ind = scope[1];

	M_StdListObject* as_list = static_cast<M_StdListObject*>(self);
	ScopedObjectLock lock(as_list);
	int index = space->unwrap_int(ind);

	int size = as_list->size();
	if (size == 0) {
		throw InterpError(space->IndexError_type(), space->wrap_str(context, "pop from empty list"));
	}

	if (index == -1) {
		M_BaseObject* result = as_list->items.back();
		context->new_local_ref(result);
		as_list->items.pop_back();
		return result;
	}

	if (index < 0) index += size;
	if (index >= size) {
		throw InterpError(space->IndexError_type(), space->wrap_str(context, "pop index out of range"));
	}

	M_BaseObject* result = as_list->items[index];
	context->new_local_ref(result);
	as_list->items.erase(as_list->items.begin() + index);

	return result;
}
