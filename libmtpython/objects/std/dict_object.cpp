#include <string>
#include <unordered_map>
#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "objects/std/dict_object.h"
#include "exceptions.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef dict_typedef(std::string("dict"), std::unordered_map<std::string, M_BaseObject*>{
	{ "__getitem__", new InterpFunctionWrapper(M_StdDictObject::__getitem__) },
	{ "__setitem__", new InterpFunctionWrapper(M_StdDictObject::__setitem__) },
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

M_BaseObject* M_StdDictObject::__getitem__(mtpython::vm::ThreadContext* context, M_BaseObject* obj, M_BaseObject* key)
{
	M_StdDictObject* as_dict = dynamic_cast<M_StdDictObject*>(obj);
	if (!as_dict) return nullptr;

	as_dict->lock();
	M_BaseObject* value = as_dict->getitem(key);
	as_dict->unlock();

	return value;
}

M_BaseObject* M_StdDictObject::__setitem__(mtpython::vm::ThreadContext* context, M_BaseObject* obj, M_BaseObject* key, M_BaseObject* value)
{
	M_StdDictObject* as_dict = dynamic_cast<M_StdDictObject*>(obj);
	if (!as_dict) return nullptr;

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
