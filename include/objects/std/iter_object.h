#ifndef _STD_ITER_OBJECT_H_
#define _STD_ITER_OBJECT_H_

#include <string>
#include <vector>
#include "objects/obj_space.h"

namespace mtpython {
namespace objects {

class M_StdTupleIterObject : public M_BaseObject {
private:
	std::size_t index;
	std::vector<M_BaseObject*> items;
public:
	M_StdTupleIterObject(const std::vector<M_BaseObject*>& items) : items(items) { index = 0; }

	virtual interpreter::Typedef* get_typedef();

	static M_BaseObject* __next__(vm::ThreadContext* context, M_BaseObject* self);
};

}
}

#endif /* _STD_ITER_OBJECT_H_ */