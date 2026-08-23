#ifndef _IOBASE_H_
#define _IOBASE_H_

#include "objects/obj_space.h"
#include "gc/garbage_collector.h"

namespace mtpython {
namespace modules {

class M_IOBase : public mtpython::objects::M_BaseObject {
private:
    mtpython::objects::M_BaseObject* dict;

public:
    M_IOBase(mtpython::objects::ObjSpace* space);

    objects::M_BaseObject* get_dict(objects::ObjSpace* space) { return dict; }

    virtual void mark_children(gc::GarbageCollector* gc)
    {
        gc->mark_object_maybe(dict);
    }
};

class M_RawIOBase : public M_IOBase {
public:
    M_RawIOBase(mtpython::objects::ObjSpace* space) : M_IOBase(space) {}
};

} // namespace modules
} // namespace mtpython

#endif /* _IOBASE_H_ */
