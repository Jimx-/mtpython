#ifndef _VM_THREAD_CONTEXT_H_
#define _VM_THREAD_CONTEXT_H_

#include "vm/native_thread.h"
#include "objects/base_object.h"

#include <deque>
#include <vector>
#include <unordered_set>
#include <functional>
#include <memory>
#include <setjmp.h>

namespace mtpython {

namespace objects {
class ObjSpace;
}

namespace interpreter {
class BaseCompiler;
class PyFrame;
class Module;
} // namespace interpreter

namespace gc {
class GCContext;
class GarbageCollector;
} // namespace gc

namespace vm {

class PyVM;

class ThreadContext {
private:
    PyVM* vm_;
    bool is_main_thread_;
    objects::ObjSpace* space_;
    gc::GarbageCollector* gc_;
    std::unique_ptr<interpreter::BaseCompiler> compiler_;
    std::unique_ptr<NativeThread> native_thread_;
    std::unique_ptr<gc::GCContext> gc_context_;

    std::deque<interpreter::PyFrame*> frame_stack_;

    jmp_buf machine_regs_;

public:
    ThreadContext(PyVM* vm, objects::ObjSpace* space, bool is_main_thread);
    ~ThreadContext();

    interpreter::BaseCompiler* get_compiler() { return compiler_.get(); }
    objects::ObjSpace* get_space() { return space_; }

    gc::GarbageCollector* get_gc() const { return gc_; }
    gc::GCContext* gc_context() const { return gc_context_.get(); }
    void bind_gc(gc::GarbageCollector* gc);

    void save_machine_context();

    void mark_roots(gc::GarbageCollector* gc);

    void enter(interpreter::PyFrame* frame);
    void leave(interpreter::PyFrame* frame);
    interpreter::PyFrame* top_frame() { return frame_stack_.back(); }

    void acquire_import_lock() {}
    void release_import_lock() {}

    static ThreadContext* current_thread()
    {
        return NativeThread::current_thread();
    }
};

} // namespace vm

} // namespace mtpython

#endif
