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

	interpreter::PyFrame* create_frame(vm::ThreadContext* context, interpreter::Code* code, M_BaseObject* globals, M_BaseObject* outer=nullptr);
	
	M_BaseObject* lookup(M_BaseObject* obj, const std::string& name);
	M_BaseObject* lookup_type_cls(M_BaseObject* obj, const std::string& attr, M_BaseObject*& where);
	M_BaseObject* lookup_type_starting_at(M_BaseObject* type, M_BaseObject* start, const std::string& name);
	M_BaseObject* issubtype(M_BaseObject* sub, M_BaseObject* type);

	M_BaseObject* type(M_BaseObject* obj) { return obj->get_class(this); }
	std::string get_type_name(M_BaseObject* obj);

	M_BaseObject* get_type_by_name(const std::string& name) { return builtin_types[name]; }

	M_BaseObject* wrap(vm::ThreadContext* context, M_BaseObject* obj) { return obj->bind_space(this); }

	M_BaseObject* wrap_int(vm::ThreadContext* context, int x);
	M_BaseObject* wrap_int(vm::ThreadContext* context, const std::string& x);

	M_BaseObject* wrap_str(vm::ThreadContext* context, const std::string& x);

	M_BaseObject* wrap_None() { return wrapped_None; }
	M_BaseObject* wrap_True() { return wrapped_True; }
	M_BaseObject* wrap_False() { return wrapped_False; }

	M_BaseObject* new_tuple(vm::ThreadContext* context, std::vector<M_BaseObject*>& items);
	M_BaseObject* new_list(vm::ThreadContext* context, std::vector<M_BaseObject*>& items);
	M_BaseObject* new_dict(vm::ThreadContext* context);
	M_BaseObject* new_set(vm::ThreadContext* context);
	M_BaseObject* new_seqiter(vm::ThreadContext* context, M_BaseObject* obj);

	void unwrap_tuple(M_BaseObject* obj, std::vector<M_BaseObject*>& list);
};

}
}

#endif
