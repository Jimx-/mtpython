#ifndef _STD_TUPLE_OBJECT_H_
#define _STD_TUPLE_OBJECT_H_

#include <string>
#include <vector>
#include "objects/obj_space.h"

namespace mtpython {
namespace objects {

#define M_STDTUPLEOBJECT(obj) (dynamic_cast<M_StdTupleObject*>(obj))

class M_StdTupleObject : public M_BaseObject {
private:
	std::vector<M_BaseObject*> items;
public:
	M_StdTupleObject() { }
	M_StdTupleObject(std::vector<M_BaseObject*>& items) : items(items) { }
	
	std::vector<M_BaseObject*>& get_items() { return items; }

	static interpreter::Typedef* _tuple_typedef();
	virtual interpreter::Typedef* get_typedef();
};

}
}

#endif /* _STD_TUPLE_OBJECT_H_ */