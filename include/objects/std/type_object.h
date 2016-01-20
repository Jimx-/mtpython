#ifndef _STD_TYPE_OBJECT_H_
#define _STD_TYPE_OBJECT_H_

#include <string>
#include <vector>
#include <unordered_map>

#include "objects/base_object.h"
#include "objects/space_cache.h"
#include "interpreter/arguments.h"
#include "vm/vm.h"

namespace mtpython {
namespace objects {

class M_StdTypeObject : public M_BaseObject {
private:
	ObjSpace* space;
	std::string name;
	std::vector<M_BaseObject*> bases;
	std::unordered_map<std::string, M_BaseObject*> dict;
	M_BaseObject* wrapped_dict;
	std::vector<M_BaseObject*> mro;
	bool _has_dict;

	void init_mro();

	void ready();
public:
	M_StdTypeObject(ObjSpace* space, std::string& name, const std::vector<M_BaseObject*>& bases, const std::unordered_map<std::string, M_BaseObject*>& dict);

	std::string get_name() { return name; }
	bool has_dict() { return _has_dict; }
	void set_has_dict(bool has_dict) { _has_dict = has_dict; }
	M_BaseObject* get_dict(ObjSpace* space);

	interpreter::Typedef* get_typedef();
	static interpreter::Typedef* _type_typedef();

	M_BaseObject* get_dict_value(ObjSpace* space, const std::string& attr);

	M_BaseObject* lookup(const std::string& name);
	M_BaseObject* lookup_starting_at(M_BaseObject* start, const std::string& name);
	M_BaseObject* lookup_cls(const std::string& attr, M_BaseObject*& cls);
	bool issubtype(M_BaseObject* type);

	static M_BaseObject* __new__(vm::ThreadContext* context, const interpreter::Arguments& args);

	static M_BaseObject* __repr__(vm::ThreadContext* context, M_BaseObject* self);
	static M_BaseObject* __mro__get(vm::ThreadContext* context, M_BaseObject* self);

	static M_BaseObject* __call__(vm::ThreadContext* context, const interpreter::Arguments& args);
	static M_BaseObject* __subclasscheck__(vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* sub);
};

class StdTypedefCache : public TypedefCache {
public:
	StdTypedefCache(ObjSpace* space) : TypedefCache(space) { }

	M_BaseObject* build(interpreter::Typedef* key);
};

}
}

#endif /* _STD_TYPE_OBJECT_H_ */
