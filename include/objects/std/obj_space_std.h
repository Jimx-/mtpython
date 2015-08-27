#ifndef _OBJ_SPACE_STD_H_
#define _OBJ_SPACE_STD_H_

#include "objects/obj_space.h"
#include <unordered_map>
#include <vector>
#include <string>

namespace mtpython {
namespace objects {

/* General-purpose object space */
class StdObjSpace : public ObjSpace {
private:
	M_BaseObject* wrapped_None;
	M_BaseObject* wrapped_True;
	M_BaseObject* wrapped_False;

	std::unordered_map<std::string, M_BaseObject*> builtin_types;
	
public:
	StdObjSpace();

	virtual interpreter::PyFrame* create_frame(vm::ThreadContext* context, interpreter::Code* code, M_BaseObject* globals);
	
	virtual M_BaseObject* lookup(M_BaseObject* obj, const std::string& name);
	virtual M_BaseObject* lookup_type_cls(M_BaseObject* obj, const std::string& attr, M_BaseObject*& where);

	virtual M_BaseObject* type(M_BaseObject* obj) { return obj->get_class(this); }

	virtual M_BaseObject* bool_type() { return builtin_types["bool"]; }
	virtual M_BaseObject* dict_type() { return builtin_types["dict"]; }
	virtual M_BaseObject* int_type() { return builtin_types["int"]; }
	virtual M_BaseObject* object_type() { return builtin_types["object"]; }
	virtual M_BaseObject* str_type() { return builtin_types["str"]; }
	virtual M_BaseObject* tuple_type() { return builtin_types["tuple"]; }
	virtual M_BaseObject* type_type() { return builtin_types["type"]; }

	virtual M_BaseObject* wrap(M_BaseObject* obj) { return obj->bind_space(this); }

	virtual M_BaseObject* wrap_int(int x);
	virtual M_BaseObject* wrap_int(const std::string& x);

	virtual M_BaseObject* wrap_str(const std::string& x);

	virtual M_BaseObject* wrap_None() { return wrapped_None; }
	virtual M_BaseObject* wrap_True() { return wrapped_True; }
	virtual M_BaseObject* wrap_False() { return wrapped_False; }

	virtual M_BaseObject* new_tuple(std::vector<M_BaseObject*>& items);
	virtual M_BaseObject* new_dict();

	virtual void unwrap_tuple(M_BaseObject* obj, std::vector<M_BaseObject*>& list);
};

}
}

#endif
