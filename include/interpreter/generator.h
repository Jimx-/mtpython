#ifndef _INTERPRETER_GENERATOR_H_
#define _INTERPRETER_GENERATOR_H_

#include "objects/base_object.h"
#include "interpreter/pyframe.h"
#include "gc/garbage_collector.h"
#include "typedef.h"
#include "vm/vm.h"
#include <string>

namespace mtpython {
namespace interpreter {

class GeneratorIterator : public objects::M_BaseObject {
private:
    PyFrame* frame;
    objects::ObjSpace* space;
    bool running;

public:
    GeneratorIterator(PyFrame*);

    Typedef* get_typedef();

    virtual void mark_children(gc::GarbageCollector* gc)
    {
        gc->mark_object(frame);
    }

    static objects::M_BaseObject* __iter__(vm::ThreadContext* context,
                                           objects::M_BaseObject* self);
    static objects::M_BaseObject* __next__(vm::ThreadContext* context,
                                           objects::M_BaseObject* self);

    static objects::M_BaseObject* send(vm::ThreadContext* context,
                                       objects::M_BaseObject* self,
                                       objects::M_BaseObject* arg);
};

} // namespace interpreter
} // namespace mtpython

#endif /* _INTERPRETER_GENERATOR_H_ */
