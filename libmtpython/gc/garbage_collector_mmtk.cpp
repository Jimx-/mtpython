#include "gc/garbage_collector_mmtk.h"
#include "vm/thread_context.h"
#include "vm/vm.h"
#include "objects/obj_space.h"

#include "mmtk.h"

#include "spdlog/spdlog.h"

namespace mtpython {

namespace gc {

template <>
GarbageCollectorMmtk* Singleton<GarbageCollectorMmtk>::m_singleton = nullptr;

namespace detail {

class Context : public GCContext {
    friend class gc::GarbageCollectorMmtk;

public:
    Context(vm::ThreadContext* thread) : GCContext(thread)
    {
        mutator_ = mmtk_bind_mutator((void*)thread);
    }

    ~Context()
    {
        if (mutator_) {
            mmtk_destroy_mutator(reinterpret_cast<MMTk_Mutator*>(mutator_));
        }
    }

private:
    MMTk_Mutator* mutator_;
};

static thread_local struct MMTk_GCThreadTLS* mmtk_gc_thread_tls;

static void mmtk_init_gc_worker_thread(struct MMTk_GCThreadTLS* gc_worker_tls)
{
    mmtk_gc_thread_tls = gc_worker_tls;
}

static struct MMTk_GCThreadTLS* mmtk_get_gc_thread_tls(void)
{
    return mmtk_gc_thread_tls;
}

static void mmtk_stop_the_world(MMTk_VMWorkerThread tls)
{
    spdlog::trace("Stopping the world for GC");

    GarbageCollectorMmtk::get_singleton().stop_the_world();
}

static void mmtk_resume_mutators(MMTk_VMWorkerThread tls)
{
    spdlog::trace("Resuming mutators after GC finishes");

    GarbageCollectorMmtk::get_singleton().resume_mutators();
}

static void mmtk_block_for_gc(MMTk_VMMutatorThread tls)
{
    spdlog::trace("Block thread for GC, tls={}", tls);

    auto* thread = vm::ThreadContext::current_thread();
    thread->save_machine_context();

    GarbageCollectorMmtk::get_singleton().block_for_gc();
}

static size_t mmtk_number_of_mutators(void) { return 1; }

static void mmtk_reset_mutator_iterator(void)
{
    GarbageCollectorMmtk::get_singleton().reset_iterator();
}

static MMTk_Mutator* mmtk_get_next_mutator(void)
{
    return (MMTk_Mutator*)GarbageCollectorMmtk::get_singleton()
        .get_next_mutator();
}

void mmtk_scan_vm_specific_roots(void)
{
    spdlog::trace("Scanning VM-specific root");

    GarbageCollectorMmtk::get_singleton().scan_vm_specific_roots();
}

void mmtk_scan_thread_roots(void) { abort(); }

void mmtk_scan_thread_root(MMTk_VMMutatorThread mutator_tls,
                           MMTk_VMWorkerThread worker_tls)
{
    spdlog::trace("Scanning thread root, tls={}", mutator_tls);

    auto* thread = static_cast<vm::ThreadContext*>(mutator_tls);

    thread->mark_roots(GarbageCollectorMmtk::get_singleton_ptr());
}

void mmtk_scan_object_ruby_style(MMTk_ObjectReference object)
{
    auto* obj = static_cast<objects::M_BaseObject*>(object);

    if (!obj->is_valid()) return;

    obj->mark_children(GarbageCollectorMmtk::get_singleton_ptr());
}

static MMTk_RubyUpcalls ruby_upcalls = {
    mmtk_init_gc_worker_thread,  mmtk_get_gc_thread_tls,
    mmtk_stop_the_world,         mmtk_resume_mutators,
    mmtk_block_for_gc,           mmtk_number_of_mutators,
    mmtk_reset_mutator_iterator, mmtk_get_next_mutator,
    mmtk_scan_vm_specific_roots, mmtk_scan_thread_roots,
    mmtk_scan_thread_root,       mmtk_scan_object_ruby_style,
};

} // namespace detail

GarbageCollectorMmtk::GarbageCollectorMmtk(vm::PyVM* vm)
    : GarbageCollector(vm), stopped_threads_(0), start_world_count_(0)
{
    MMTk_Builder* mmtk_builder = mmtk_builder_default();
    RubyBindingOptions ruby_binding_options;

    mmtk_builder_set_plan(mmtk_builder, "MarkSweep");

    mmtk_builder_set_fixed_heap_size(mmtk_builder, 32 * 1024 * 1024);

    ruby_binding_options.ractor_check_mode = false;
    ruby_binding_options.suffix_size = 0;

    mmtk_init_binding(mmtk_builder, &ruby_binding_options,
                      &detail::ruby_upcalls);

    mmtk_initialize_collection(vm::ThreadContext::current_thread());
}

std::unique_ptr<GCContext>
GarbageCollectorMmtk::create_context(vm::ThreadContext* thread)
{
    return std::make_unique<detail::Context>(thread);
}

void* GarbageCollectorMmtk::allocate(size_t size)
{
    auto* context = static_cast<detail::Context*>(
        vm::ThreadContext::current_thread()->gc_context());
    size_t prefix_size = MMTK_OBJREF_OFFSET;
    size_t mmtk_alloc_size = prefix_size + size;

    void* addr = mmtk_alloc(context->mutator_, mmtk_alloc_size,
                            MMTK_MIN_OBJ_ALIGN, 0, 0);

    *(size_t*)addr = size;

    void* obj = (char*)addr + prefix_size;
    mmtk_post_alloc(context->mutator_, obj, mmtk_alloc_size, 0);

    mmtk_add_finalizer(obj);

    return obj;
}

void GarbageCollectorMmtk::start_gc()
{
    mmtk_handle_user_collection_request(vm::ThreadContext::current_thread());
}

void GarbageCollectorMmtk::reset_iterator()
{
    threads_.clear();
    threads_.push_back(vm_->get_main_thread());
    thread_iter_ = threads_.begin();
}

void* GarbageCollectorMmtk::get_next_mutator()
{
    if (thread_iter_ == threads_.end()) return nullptr;

    auto* thread = *thread_iter_;
    thread_iter_++;
    return static_cast<detail::Context*>(thread->gc_context())->mutator_;
}

void GarbageCollectorMmtk::block_for_gc()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);

        stopped_threads_++;
        if (stopped_threads_ == 1) stop_cond_.notify_all();

        size_t count = start_world_count_;

        start_cond_.wait(lock, [=] { return start_world_count_ >= count + 1; });

        stopped_threads_--;
    }

    call_finalizers(false);
}

void GarbageCollectorMmtk::call_finalizers(bool on_exit)
{
    if (on_exit) {

    } else {
        void* resurrected;

        while ((resurrected = mmtk_get_finalized_object()) != NULL) {
            objects::M_BaseObject* obj = (objects::M_BaseObject*)resurrected;
            obj->~M_BaseObject();
        }
    }
}

void GarbageCollectorMmtk::stop_the_world()
{
    std::unique_lock<std::mutex> lock(mutex_);
    stop_cond_.wait(lock, [=] { return stopped_threads_ == 1; });
}

void GarbageCollectorMmtk::resume_mutators()
{
    std::unique_lock<std::mutex> lock(mutex_);
    start_world_count_++;
    start_cond_.notify_all();
}

void GarbageCollectorMmtk::mark_object(objects::M_BaseObject* obj)
{
    detail::mmtk_gc_thread_tls->object_closure.c_function(
        detail::mmtk_gc_thread_tls->object_closure.rust_closure,
        detail::mmtk_gc_thread_tls->gc_context, obj);
}

void GarbageCollectorMmtk::scan_vm_specific_roots() { vm_->mark_roots(); }

bool GarbageCollectorMmtk::is_heap_pointer(void* ptr)
{
    if (!ptr) return false;

    if ((uintptr_t)ptr % sizeof(void*) != 0) return false;

    return mmtk_is_mmtk_object(ptr);
}

} // namespace gc

} // namespace mtpython
