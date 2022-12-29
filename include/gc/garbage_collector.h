#ifndef _GC_GARBAGE_COLLECTOR_H_
#define _GC_GARBAGE_COLLECTOR_H_

#include "vm/thread_context.h"
#include "gc/gc_context.h"

#include <cstddef>
#include <memory>

namespace mtpython {
namespace gc {

class GarbageCollector {
public:
    GarbageCollector(vm::PyVM* vm) : vm_(vm) {}

    virtual std::unique_ptr<GCContext>
    create_context(vm::ThreadContext* thread) = 0;

    virtual void* allocate(size_t size) = 0;

    virtual void start_gc() {}

    virtual void mark_object(objects::M_BaseObject* obj) {}

    void mark_object_maybe(objects::M_BaseObject* obj)
    {
        if (is_heap_pointer(obj) && obj->is_valid()) {
            mark_object(obj);
        }
    }

protected:
    vm::PyVM* vm_;

    virtual bool is_heap_pointer(void* ptr) = 0;
};

} // namespace gc
} // namespace mtpython

#endif
