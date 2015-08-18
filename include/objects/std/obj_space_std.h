#ifndef _OBJ_SPACE_STD_H_
#define _OBJ_SPACE_STD_H_

#include "objects/obj_space.h"
#include <unordered_map>

namespace mtpython {
namespace objects {

/* General-purpose object space */
class StdObjSpace : public ObjSpace {
private:
	std::unordered_map<std::string, M_BaseObject*> builtin_types;
	
public:
	StdObjSpace();

	virtual interpreter::PyFrame* create_frame(vm::ThreadContext* context, interpreter::Code* code, M_BaseObject* globals);
	
	virtual M_BaseObject* lookup_type_cls(M_BaseObject* obj, std::string& attr, M_BaseObject*& where);

	virtual M_BaseObject* type(M_BaseObject* obj) { return obj->get_class(this); }

	virtual M_BaseObject* wrap(M_BaseObject* obj) { return obj->bind_space(this); }

	virtual M_BaseObject* wrap_int(int x);
	virtual M_BaseObject* wrap_int(std::string& x);

	virtual M_BaseObject* wrap_str(std::string& x);
};

}
}

#endif
