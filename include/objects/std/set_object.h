#ifndef _STD_SET_OBJECT_H_
#define _STD_SET_OBJECT_H_

#include <string>
#include <unordered_set>
#include "objects/obj_space.h"
#include "objects/std/dict_object.h"

namespace mtpython {
namespace objects {

class M_StdSetObject : public M_BaseObject {
private:
	std::unordered_set<M_BaseObject*, M_StdObjectHasher, M_StdObjectEq> set;
public:
	M_StdSetObject(ObjSpace* space) : set(5, M_StdObjectHasher(space), M_StdObjectEq(space)) { }

	static objects::M_BaseObject* __new__(vm::ThreadContext* context, const interpreter::Arguments& args);
	static objects::M_BaseObject* __init__(vm::ThreadContext* context, const interpreter::Arguments& args);
	/*
	static M_BaseObject* __repr__(vm::ThreadContext* context, M_BaseObject* self);
	static M_BaseObject* __contains__(vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* obj);
	*/
	
	static M_BaseObject* M_StdSetObject::add(vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* item);

	static interpreter::Typedef* _set_typedef();
	interpreter::Typedef* get_typedef();
};

}
}

#endif /* _STD_SET_OBJECT_H_ */
