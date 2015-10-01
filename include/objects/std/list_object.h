#ifndef _STD_LIST_OBJECT_H_
#define _STD_LIST_OBJECT_H_

#include <string>
#include <vector>
#include "objects/obj_space.h"

namespace mtpython {
namespace objects {

#define M_STDLISTOBJECT(obj) (dynamic_cast<M_StdListObject*>(obj))

class M_StdListObject : public M_BaseObject {
private:
	std::vector<M_BaseObject*> items;
public:
	M_StdListObject() { }
	M_StdListObject(const std::vector<M_BaseObject*>& items) : items(items) { }
	
	std::vector<M_BaseObject*>& get_items() { return items; }
	virtual void unpack_iterable(ObjSpace* space, std::vector<M_BaseObject*>& list) { list.insert(list.end(), items.begin(), items.end()); }

	static interpreter::Typedef* _list_typedef();
	virtual interpreter::Typedef* get_typedef();

	static M_BaseObject* __len__(vm::ThreadContext* context, M_BaseObject* self);
	static M_BaseObject* __repr__(vm::ThreadContext* context, M_BaseObject* self);

	static M_BaseObject* append(vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* item);
	static M_BaseObject* extend(vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* iterable);
};

}
}

#endif /* _STD_LIST_OBJECT_H_ */