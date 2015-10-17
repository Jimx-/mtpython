#ifndef _STD_ITER_OBJECT_H_
#define _STD_ITER_OBJECT_H_

#include <string>
#include <vector>
#include "objects/obj_space.h"

namespace mtpython {
namespace objects {

class M_StdSeqIterObject : public M_BaseObject {
private:
	int index;
	M_BaseObject* obj;
public:
	M_StdSeqIterObject(M_BaseObject* obj, int index = 0) : obj(obj), index(index) { }
	interpreter::Typedef* get_typedef();

	static M_BaseObject* __next__(vm::ThreadContext* context, M_BaseObject* self);
};

class M_StdTupleIterObject : public M_BaseObject {
private:
	std::size_t index;
	std::vector<M_BaseObject*> items;
public:
	M_StdTupleIterObject(const std::vector<M_BaseObject*>& items) : items(items) { index = 0; }

	interpreter::Typedef* get_typedef();

	static M_BaseObject* __next__(vm::ThreadContext* context, M_BaseObject* self);
};

}
}

#endif /* _STD_ITER_OBJECT_H_ */