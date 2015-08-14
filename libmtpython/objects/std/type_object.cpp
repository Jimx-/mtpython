#include "objects/std/type_object.h"
#include "objects/obj_space.h"

using namespace mtpython::objects;

static mtpython::interpreter::Typedef type_typedef(std::string("type"), std::unordered_map<std::string, M_BaseObject*>({}));

StdTypeObject::StdTypeObject(ObjSpace* space, std::string& name, std::vector<M_BaseObject*>& bases, std::unordered_map<std::string, M_BaseObject*>& dict) :
			space(space), name(name), bases(bases), dict(dict)
{

}

mtpython::interpreter::Typedef* StdTypeObject::_type_typedef()
{
	return &type_typedef;
}

M_BaseObject* StdTypedefCache::build(mtpython::interpreter::Typedef* key)
{
	std::unordered_map<std::string, M_BaseObject*>& dict = key->get_dict();

	std::vector<M_BaseObject*> wrapped_bases;

	std::unordered_map<std::string, M_BaseObject*> wrapped_dict;
	for (auto it = dict.begin(); it != dict.end(); it++) {
		wrapped_dict[it->first] = space->wrap(it->second);
	}

	StdTypeObject* wrapped_type = new StdTypeObject(space, key->get_name(), wrapped_bases, wrapped_dict);

	return wrapped_type;
}
