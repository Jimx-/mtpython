#include "gc/collected_heap.h"

using namespace mtpython::gc;

size_t CollectedHeap::capacity()
{
    return 0;
}

size_t CollectedHeap::used()
{
    return 0;
}

void* CollectedHeap::allocate_mem(size_t size)
{
    return ::operator new(size);
}

void CollectedHeap::post_allocate_obj(void* obj)
{

}

