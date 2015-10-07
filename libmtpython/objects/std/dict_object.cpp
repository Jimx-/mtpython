#include <string>
#include <unordered_map>
#include <assert.h>

#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "interpreter/error.h"
#include "objects/std/dict_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef dict_typedef("dict", {
	{ "__repr__", new InterpFunctionWrapper("__repr__", M_StdDictObject::__repr__) },
	{ "__getitem__", new InterpFunctionWrapper("__getitem__", M_StdDictObject::__getitem__) },
	{ "__setitem__", new InterpFunctionWrapper("__setitem__", M_StdDictObject::__setitem__) },
	{ "__contains__", new InterpFunctionWrapper("__contains__", M_StdDictObject::__contains__) },
	{ "keys", new InterpFunctionWrapper("keys", M_StdDictObject::keys) },
});

M_BaseObject* M_StdDictObject::getitem(M_BaseObject* key)
{
	auto got = dict.find(key);
	if (got == dict.end()) return nullptr;

	return got->second;
}

void M_StdDictObject::setitem(M_BaseObject* key, M_BaseObject* value)
{
	dict[key] = value;
}

M_BaseObject* M_StdDictObject::__repr__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	ObjSpace* space = context->get_space();
	M_StdDictObject* as_dict = dynamic_cast<M_StdDictObject*>(self);
	assert(as_dict);
	std::string str;

	as_dict->lock();
	str += "{";
	int i = 0;
	for (auto& item : as_dict->dict) {
		if (i > 0) str += ", ";
		M_BaseObject* repr_item = space->repr(item.first);
		str += space->unwrap_str(repr_item);
		SAFE_DELETE(repr_item);
		str += ": ";
		repr_item = space->repr(item.second);
		str += space->unwrap_str(repr_item);
		SAFE_DELETE(repr_item);
		i++;
	}
	str += "}";
	as_dict->unlock();

	return space->wrap_str(context, str);
}

M_BaseObject* M_StdDictObject::__getitem__(mtpython::vm::ThreadContext* context, M_BaseObject* obj, M_BaseObject* key)
{
	ObjSpace* space = context->get_space();
	M_StdDictObject* as_dict = dynamic_cast<M_StdDictObject*>(obj);
	assert(as_dict);

	as_dict->lock();
	M_BaseObject* value = as_dict->getitem(key);
	as_dict->unlock();

	if (!value) throw InterpError(space->KeyError_type(), key);
	return value;
}

M_BaseObject* M_StdDictObject::__setitem__(mtpython::vm::ThreadContext* context, M_BaseObject* obj, M_BaseObject* key, M_BaseObject* value)
{
	M_StdDictObject* as_dict = dynamic_cast<M_StdDictObject*>(obj);
	assert(as_dict);

	as_dict->lock();
	as_dict->setitem(key, value);
	as_dict->unlock();

	return nullptr;
}

mtpython::interpreter::Typedef* M_StdDictObject::_dict_typedef()
{
	return &dict_typedef;
}

mtpython::interpreter::Typedef* M_StdDictObject::get_typedef()
{
	return &dict_typedef;
}

M_BaseObject* M_StdDictObject::__contains__(mtpython::vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* obj)
{
	M_StdDictObject* as_dict = static_cast<M_StdDictObject*>(self);

	as_dict->lock();
	M_BaseObject* result = context->get_space()->new_bool(as_dict->dict.find(obj) != as_dict->dict.end());
	as_dict->unlock();

	return result;
}

M_BaseObject* M_StdDictObject::keys(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	M_StdDictObject* as_dict = static_cast<M_StdDictObject*>(self);
	std::vector<M_BaseObject*> keys;
	
	as_dict->lock();
	for (const auto& iter : as_dict->dict) {
		keys.push_back(iter.first);
	}
	as_dict->unlock();

	return context->get_space()->new_tuple(context, keys);
}
