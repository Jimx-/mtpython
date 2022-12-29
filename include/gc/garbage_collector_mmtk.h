#ifndef _GC_MMTK_GARBAGE_COLLECTOR_H_
#define _GC_MMTK_GARBAGE_COLLECTOR_H_

#include "singleton.h"
#include "gc/garbage_collector.h"

#include <vector>
#include <mutex>
#include <condition_variable>

namespace mtpython {
namespace gc {

class GarbageCollectorMmtk : public GarbageCollector,
                             public Singleton<GarbageCollectorMmtk> {
public:
    GarbageCollectorMmtk(vm::PyVM* vm);

    virtual std::unique_ptr<GCContext>
    create_context(vm::ThreadContext* thread);

    virtual void* allocate(size_t size);

    virtual void start_gc();

    void reset_iterator();
    void* get_next_mutator();

    void block_for_gc();
    void stop_the_world();
    void resume_mutators();

    virtual void mark_object(objects::M_BaseObject* obj);

    void scan_vm_specific_roots();

private:
    std::mutex mutex_;

    std::vector<vm::ThreadContext*> threads_;
    std::vector<vm::ThreadContext*>::iterator thread_iter_;

    unsigned int stopped_threads_;
    unsigned int start_world_count_;
    std::condition_variable stop_cond_;
    std::condition_variable start_cond_;

    void call_finalizers(bool on_exit);

    virtual bool is_heap_pointer(void* ptr);
};

} // namespace gc
} // namespace mtpython

#endif
