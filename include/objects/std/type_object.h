#ifndef _STD_TYPE_OBJECT_H_
#define _STD_TYPE_OBJECT_H_

#include "objects/base_object.h"
#include "objects/space_cache.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace mtpython {
namespace objects {

class StdTypeObject : public M_BaseObject {
private:
	ObjSpace* space;
	std::string name;
	std::vector<M_BaseObject*> bases;
	std::unordered_map<std::string, M_BaseObject*> dict;
	std::vector<M_BaseObject*> mro;

	void init_mro();
public:
	StdTypeObject(ObjSpace* space, std::string& name, std::vector<M_BaseObject*>& bases, std::unordered_map<std::string, M_BaseObject*>& dict);

	static interpreter::Typedef* _type_typedef();

	virtual M_BaseObject* get_dict_value(ObjSpace* space, std::string& attr);

	virtual M_BaseObject* lookup(std::string& name);
	virtual M_BaseObject* lookup_cls(std::string& attr, M_BaseObject*& cls);
};

class StdTypedefCache : public TypedefCache {
public:
	StdTypedefCache(ObjSpace* space) : TypedefCache(space) { }

	M_BaseObject* build(interpreter::Typedef* key);
};

}
}

#endif /* _STD_TYPE_OBJECT_H_ */
