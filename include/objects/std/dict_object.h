#ifndef _STD_DICT_OBJECT_H_
#define _STD_DICT_OBJECT_H_

#include <string>
#include <unordered_map>
#include "objects/obj_space.h"
#include "gc/garbage_collector.h"

namespace mtpython {
namespace objects {

class M_StdObjectHasher {
private:
    ObjSpace* space;

public:
    M_StdObjectHasher(ObjSpace* space) { this->space = space; }
    std::size_t operator()(M_BaseObject* const& key) const
    {
        return space->i_hash(key);
    }
};

class M_StdObjectEq {
private:
    ObjSpace* space;

public:
    M_StdObjectEq(ObjSpace* space) { this->space = space; }
    bool operator()(M_BaseObject* const& lhs, M_BaseObject* const& rhs) const
    {
        return space->i_eq(lhs, rhs);
    }
};

class M_StdDictObject : public M_BaseObject {
private:
    std::unordered_map<M_BaseObject*, M_BaseObject*, M_StdObjectHasher,
                       M_StdObjectEq>
        dict;

public:
    M_StdDictObject(ObjSpace* space)
        : dict(5, M_StdObjectHasher(space), M_StdObjectEq(space))
    {}

    M_BaseObject* getitem(M_BaseObject* key);
    void setitem(M_BaseObject* key, M_BaseObject* value);

    virtual void mark_chidren(gc::GarbageCollector* gc)
    {
        for (const auto& [k, v] : dict) {
            gc->mark_object(k);
            gc->mark_object(v);
        }
    }

    static M_BaseObject* __repr__(vm::ThreadContext* context,
                                  M_BaseObject* self);
    static M_BaseObject* __getitem__(mtpython::vm::ThreadContext* context,
                                     M_BaseObject* obj, M_BaseObject* key);
    static M_BaseObject* __setitem__(mtpython::vm::ThreadContext* context,
                                     M_BaseObject* obj, M_BaseObject* key,
                                     M_BaseObject* value);
    static M_BaseObject* __contains__(vm::ThreadContext* context,
                                      M_BaseObject* self, M_BaseObject* obj);

    static M_BaseObject* keys(vm::ThreadContext* context, M_BaseObject* self);
    static M_BaseObject* values(vm::ThreadContext* context, M_BaseObject* self);
    static M_BaseObject* items(vm::ThreadContext* context, M_BaseObject* self);

    static interpreter::Typedef* _dict_typedef();
    virtual interpreter::Typedef* get_typedef();
};

} // namespace objects
} // namespace mtpython

#endif /* _STD_DICT_OBJECT_H_ */
