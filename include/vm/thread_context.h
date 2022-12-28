#ifndef _VM_THREAD_CONTEXT_H_
#define _VM_THREAD_CONTEXT_H_

#include "objects/obj_space.h"
#include "gc/heap.h"
#include "vm/native_thread.h"

#include <stack>
#include <vector>
#include <unordered_set>
#include <functional>
#include <memory>

namespace mtpython {

namespace interpreter {
class BaseCompiler;
class PyFrame;
class Module;
} // namespace interpreter

namespace vm {

class PyVM;

class ThreadContext {
private:
    class LocalFrame {
    private:
        std::unordered_set<objects::M_BaseObject*> local_refs;

    public:
        void new_local_ref(objects::M_BaseObject* obj);
        void delete_local_ref(objects::M_BaseObject* obj);
    };

    PyVM* vm_;
    bool is_main_thread_;
    objects::ObjSpace* space_;
    interpreter::BaseCompiler* compiler_;
    std::unique_ptr<NativeThread> native_thread_;

    std::stack<interpreter::PyFrame*> frame_stack_;
    std::vector<LocalFrame*> local_frame_stack_;
    LocalFrame* top_local_frame_;

public:
    ThreadContext(PyVM* vm, objects::ObjSpace* space, bool is_main_thread);
    ~ThreadContext();

    interpreter::BaseCompiler* get_compiler() { return compiler_; }
    objects::ObjSpace* get_space() { return space_; }
    gc::Heap* heap() { return nullptr; }

    void enter(interpreter::PyFrame* frame);
    void leave(interpreter::PyFrame* frame);
    interpreter::PyFrame* top_frame() { return frame_stack_.top(); }

    void push_local_frame();
    objects::M_BaseObject* pop_local_frame(objects::M_BaseObject* result);

    void new_local_ref(objects::M_BaseObject* obj);
    void delete_local_ref(objects::M_BaseObject* obj);

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
