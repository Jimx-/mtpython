#ifndef _STD_OBJECT_OBJECT_H_
#define _STD_OBJECT_OBJECT_H_

#include <string>
#include "objects/base_object.h"
#include "gc/garbage_collector.h"
#include "vm/vm.h"

namespace mtpython {
namespace objects {

class M_StdObjectObject : public M_BaseObject {
private:
    M_BaseObject* dict;
    M_BaseObject* obj_type;

public:
    M_StdObjectObject(M_BaseObject* type) : obj_type(type) { dict = nullptr; }
    interpreter::Typedef* get_typedef();
    static interpreter::Typedef* _object_typedef();
    M_BaseObject* get_class(ObjSpace* space) { return obj_type; }
    void set_class(ObjSpace* space, M_BaseObject* type) { obj_type = type; }

    M_BaseObject* get_dict(ObjSpace* space) { return dict; }
    void set_dict(M_BaseObject* dict) { this->dict = dict; }

    virtual void mark_children(gc::GarbageCollector* gc)
    {
        gc->mark_object(dict);
        gc->mark_object(obj_type);
    }

    static objects::M_BaseObject* __new__(vm::ThreadContext* context,
                                          const interpreter::Arguments& args);
    static objects::M_BaseObject* __init__(vm::ThreadContext* context,
                                           const interpreter::Arguments& args);

    static M_BaseObject* __str__(vm::ThreadContext* context, M_BaseObject* obj);
    static M_BaseObject* __repr__(vm::ThreadContext* context,
                                  M_BaseObject* obj);

    static M_BaseObject* __getattribute__(vm::ThreadContext* context,
                                          M_BaseObject* obj,
                                          M_BaseObject* attr);
    static M_BaseObject* __setattr__(vm::ThreadContext* context,
                                     M_BaseObject* obj, M_BaseObject* attr,
                                     M_BaseObject* value);
    static M_BaseObject* __delattr__(vm::ThreadContext* context,
                                     M_BaseObject* obj, M_BaseObject* attr);
    static M_BaseObject* __subclasshook__(vm::ThreadContext* context,
                                          const interpreter::Arguments& args);

    static M_BaseObject* __dict__get(mtpython::vm::ThreadContext* context,
                                     M_BaseObject* obj);
    static void __dict__set(vm::ThreadContext* context, M_BaseObject* obj,
                            M_BaseObject* value);
    static M_BaseObject* __class__get(mtpython::vm::ThreadContext* context,
                                      M_BaseObject* obj);
};

} // namespace objects
} // namespace mtpython

#endif /* _STD_OBJECT_OBJECT_H_ */
