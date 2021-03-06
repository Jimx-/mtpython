#ifndef _STD_TUPLE_OBJECT_H_
#define _STD_TUPLE_OBJECT_H_

#include <string>
#include <vector>
#include "objects/obj_space.h"

namespace mtpython {
namespace objects {

#define M_STDTUPLEOBJECT(obj) (static_cast<M_StdTupleObject*>(obj))

class M_StdTupleObject : public M_BaseObject {
private:
    std::vector<M_BaseObject*> items;

public:
    M_StdTupleObject() {}
    M_StdTupleObject(const std::vector<M_BaseObject*>& items) : items(items) {}

    std::vector<M_BaseObject*>& get_items() { return items; }

    static interpreter::Typedef* _tuple_typedef();
    virtual interpreter::Typedef* get_typedef();

    static M_BaseObject* __iter__(vm::ThreadContext* context,
                                  M_BaseObject* self);
    static M_BaseObject* __len__(vm::ThreadContext* context,
                                 M_BaseObject* self);
    static M_BaseObject* __repr__(vm::ThreadContext* context,
                                  M_BaseObject* self);
    static M_BaseObject* __getitem__(vm::ThreadContext* context,
                                     M_BaseObject* obj, M_BaseObject* key);
    static M_BaseObject* __contains__(vm::ThreadContext* context,
                                      M_BaseObject* self, M_BaseObject* obj);
};

} // namespace objects
} // namespace mtpython

#endif /* _STD_TUPLE_OBJECT_H_ */