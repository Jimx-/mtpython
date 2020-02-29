#ifndef _HEAP_H_
#define _HEAP_H_

#include <cstddef>

namespace mtpython {
namespace gc {

struct HeapAllocation {};

class Heap {
public:
    ~Heap() {}

    virtual size_t capacity() = 0;
    virtual size_t used() = 0;

    virtual void* allocate_mem(size_t size) = 0;
    virtual void post_allocate_obj(void* obj) = 0;
};

} // namespace gc
} // namespace mtpython

#endif
