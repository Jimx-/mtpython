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
    M_BaseObject* wrapped_NotImplemented;

    std::unordered_map<std::string, M_BaseObject*> builtin_types;

    virtual void initialize();

public:
    StdObjSpace();

    virtual void mark_roots(gc::GarbageCollector* gc);

    interpreter::PyFrame* create_frame(vm::ThreadContext* context,
                                       interpreter::Code* code,
                                       M_BaseObject* globals,
                                       M_BaseObject* outer = nullptr);

    M_BaseObject* lookup(M_BaseObject* obj, const std::string& name);
    M_BaseObject* lookup_type_cls(M_BaseObject* obj, const std::string& attr,
                                  M_BaseObject*& where);
    M_BaseObject* lookup_type_starting_at(M_BaseObject* type,
                                          M_BaseObject* start,
                                          const std::string& name);
    bool i_isinstance(M_BaseObject* inst, M_BaseObject* type);
    bool i_issubtype(M_BaseObject* sub, M_BaseObject* type);

    M_BaseObject* type(M_BaseObject* obj) { return obj->get_class(this); }
    std::string get_type_name(M_BaseObject* obj);

    M_BaseObject* get_type_by_name(const std::string& name);

    M_BaseObject* wrap(vm::ThreadContext* context, M_BaseObject* obj)
    {
        return obj->bind_space(this);
    }

    M_BaseObject* wrap_int(vm::ThreadContext* context, int x);
    M_BaseObject* wrap_int(vm::ThreadContext* context, const std::string& x);

    M_BaseObject* wrap_str(vm::ThreadContext* context, const std::string& x);

    M_BaseObject* wrap_None() { return wrapped_None; }
    M_BaseObject* wrap_True() { return wrapped_True; }
    M_BaseObject* wrap_False() { return wrapped_False; }
    M_BaseObject* wrap_NotImplemented() { return wrapped_NotImplemented; }

    int i_get_index(M_BaseObject* obj, M_BaseObject* exc, M_BaseObject* descr);

    M_BaseObject* new_tuple(vm::ThreadContext* context,
                            const std::vector<M_BaseObject*>& items);
    M_BaseObject* new_list(vm::ThreadContext* context,
                           const std::vector<M_BaseObject*>& items);
    M_BaseObject* new_dict(vm::ThreadContext* context);
    M_BaseObject* new_set(vm::ThreadContext* context);
    M_BaseObject* new_seqiter(vm::ThreadContext* context, M_BaseObject* obj);

    void unwrap_tuple(M_BaseObject* obj, std::vector<M_BaseObject*>& list);
};

} // namespace objects
} // namespace mtpython

#endif
