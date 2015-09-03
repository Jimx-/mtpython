#ifndef _STD_TYPE_OBJECT_H_
#define _STD_TYPE_OBJECT_H_

#include <string>
#include <vector>
#include <unordered_map>

#include "objects/base_object.h"
#include "objects/space_cache.h"
#include "vm/vm.h"

namespace mtpython {
namespace objects {

class M_StdTypeObject : public M_BaseObject {
private:
	ObjSpace* space;
	std::string name;
	std::vector<M_BaseObject*> bases;
	std::unordered_map<std::string, M_BaseObject*> dict;
	std::vector<M_BaseObject*> mro;

	void init_mro();
public:
	M_StdTypeObject(ObjSpace* space, std::string& name, std::vector<M_BaseObject*>& bases, std::unordered_map<std::string, M_BaseObject*>& dict);

	virtual interpreter::Typedef* get_typedef();
	static interpreter::Typedef* _type_typedef();

	virtual M_BaseObject* get_dict_value(ObjSpace* space, const std::string& attr);

	virtual M_BaseObject* lookup(const std::string& name);
	virtual M_BaseObject* lookup_cls(const std::string& attr, M_BaseObject*& cls);

	static M_BaseObject* __repr__(vm::ThreadContext* context, M_BaseObject* self);
	static M_BaseObject* __mro__get(vm::ThreadContext* context, M_BaseObject* self);
};

class StdTypedefCache : public TypedefCache {
public:
	StdTypedefCache(ObjSpace* space) : TypedefCache(space) { }

	M_BaseObject* build(interpreter::Typedef* key);
};

}
}

#endif /* _STD_TYPE_OBJECT_H_ */
