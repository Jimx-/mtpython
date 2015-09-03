#include <list>

#include "objects/std/type_object.h"
#include "objects/std/object_object.h"
#include "interpreter/gateway.h"
#include "interpreter/descriptor.h"


using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef type_typedef("type", {
	{ "__repr__", new InterpFunctionWrapper("__repr__", M_StdTypeObject::__repr__) },
	{ "__mro__", new GetSetDescriptor(M_StdTypeObject::__mro__get) },
});

M_StdTypeObject::M_StdTypeObject(ObjSpace* space, std::string& name, std::vector<M_BaseObject*>& bases, std::unordered_map<std::string, M_BaseObject*>& dict) :
			space(space), name(name), bases(bases), dict(dict)
{
	init_mro();
}

mtpython::interpreter::Typedef* M_StdTypeObject::get_typedef()
{
	return &type_typedef;
}

mtpython::interpreter::Typedef* M_StdTypeObject::_type_typedef()
{
	return &type_typedef;
}

void M_StdTypeObject::init_mro()
{
	std::list<std::vector<M_BaseObject*>> orderlist;
	std::vector<M_BaseObject*> _bases;
	_bases.push_back(this);
	for (auto base : bases) {
		_bases.push_back(base);
		M_StdTypeObject* typeobj = dynamic_cast<M_StdTypeObject*>(base);
		if (!typeobj) continue;

		orderlist.push_back(typeobj->mro);
	}

	orderlist.push_back(_bases);

	M_BaseObject* candidate;
	while (true) {
		candidate = nullptr;
		for (auto& candidates : orderlist) {
			if (candidates.size() > 0) {
				candidate = candidates[0];

				bool ok = true;
				for (auto& lst : orderlist) {
					if (lst.size() <= 1) continue;
					for (auto it = lst.begin() + 1; it != lst.end(); it++) {
						if (*it == candidate) ok = false;
					}
				}

				if (ok) break;
			}
		}

		if (!candidate) return;

		mro.push_back(candidate);
		for (auto it = orderlist.rbegin(); it != orderlist.rend(); it++) {
			if ((*it).size() == 0) continue;
			if ((*it)[0] == candidate) (*it).erase((*it).begin());
		}
	}
}

M_BaseObject* M_StdTypeObject::get_dict_value(ObjSpace* space, const std::string& attr)
{
	auto got = dict.find(attr);
	if (got == dict.end()) return nullptr;

	return got->second;
}

M_BaseObject* M_StdTypeObject::lookup(const std::string& name)
{
	for (auto base : mro) {
		M_BaseObject* value = base->get_dict_value(space, name);
		if (value) {
			return value;
		}
	}

	return nullptr;
}

M_BaseObject* M_StdTypeObject::lookup_cls(const std::string& attr, M_BaseObject*& cls)
{
	for (auto base : mro) {
		M_BaseObject* value = base->get_dict_value(space, attr);
		if (value) {
			cls = base;
			return value;
		}
	}

	return nullptr;
}

M_BaseObject* StdTypedefCache::build(mtpython::interpreter::Typedef* key)
{
	std::unordered_map<std::string, M_BaseObject*>& dict = key->get_dict();

	std::vector<M_BaseObject*> wrapped_bases;

	if (key == M_StdObjectObject::_object_typedef()) 
		wrapped_bases.clear();
	else {
		std::vector<mtpython::interpreter::Typedef*>& typedef_bases = key->get_bases();
		if (typedef_bases.size() != 0) {
			for (mtpython::interpreter::Typedef* def : typedef_bases) {
				wrapped_bases.push_back(space->get_typeobject(def));
			}
		} else {
			wrapped_bases.push_back(space->get_typeobject(M_StdObjectObject::_object_typedef()));
		}
	}

	std::unordered_map<std::string, M_BaseObject*> wrapped_dict;
	for (auto it = dict.begin(); it != dict.end(); it++) {
		wrapped_dict[it->first] = space->wrap(it->second);
	}

	M_StdTypeObject* wrapped_type = new M_StdTypeObject(space, key->get_name(), wrapped_bases, wrapped_dict);

	return wrapped_type;
}

M_BaseObject* M_StdTypeObject::__repr__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	std::string str;
	str = "<class '";
	M_StdTypeObject* as_type = static_cast<M_StdTypeObject*>(self);
	str += as_type->name;
	str += "'>";

	return context->get_space()->wrap_str(str);
}

M_BaseObject* M_StdTypeObject::__mro__get(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	M_StdTypeObject* as_type = static_cast<M_StdTypeObject*>(self);
	return context->get_space()->new_tuple(as_type->mro);
}
