#ifndef _COLLECTED_HEAP_H_
#define _COLLECTED_HEAP_H_

#include "gc/heap.h"

namespace mtpython {
namespace gc {

class CollectedHeap : public Heap {
public:
    virtual size_t capacity();
    virtual size_t used();

    virtual void* allocate_mem(size_t size);

    virtual void post_allocate_obj(void* obj);
};

} // namespace gc
} // namespace mtpython

#endif
