#ifndef _TEXTIO_H_
#define _TEXTIO_H_

#include "objects/obj_space.h"
#include "modules/_io/iobase.h"

namespace mtpython {
namespace modules {

class M_TextIOBase : public M_IOBase {
public:
    M_TextIOBase(mtpython::objects::ObjSpace* space) : M_IOBase(space) {}
};

class M_TextIOWrapper : public M_TextIOBase {
private:
    objects::M_BaseObject* buffer;
    objects::M_BaseObject* encoding;

public:
    M_TextIOWrapper(objects::ObjSpace* space)
        : M_TextIOBase(space), buffer(nullptr), encoding(nullptr)
    {}

    interpreter::Typedef* get_typedef();

    virtual void mark_children(gc::GarbageCollector* gc)
    {
        M_TextIOBase::mark_children(gc);
        gc->mark_object_maybe(buffer);
        gc->mark_object_maybe(encoding);
    }

    static objects::M_BaseObject* __new__(vm::ThreadContext* context,
                                          const interpreter::Arguments& args);
    static objects::M_BaseObject* __init__(vm::ThreadContext* context,
                                           const interpreter::Arguments& args);
    static objects::M_BaseObject* __repr__(vm::ThreadContext* context,
                                           objects::M_BaseObject* self);

    static objects::M_BaseObject* name_get(vm::ThreadContext* context,
                                           objects::M_BaseObject* self);
    static objects::M_BaseObject* buffer_get(vm::ThreadContext* context,
                                             objects::M_BaseObject* self);
};

} // namespace modules
} // namespace mtpython

#endif /* _TEXTIO_H_ */
