#include <assert.h>
#include "macros.h"
#include "vm/vm.h"
#include "interpreter/compiler.h"
#include "vm/native_thread_posix.h"
#include "gc/garbage_collector.h"
#include "interpreter/pyframe.h"

using namespace mtpython::vm;
using namespace mtpython::objects;

ThreadContext::ThreadContext(PyVM* vm, ObjSpace* space, bool is_main_thread)
    : vm_(vm), is_main_thread_(is_main_thread), space_(space),
      gc_context_(nullptr)
{
    compiler_.reset(space->get_compiler(this));

    native_thread_ = std::make_unique<NativeThreadPOSIX>(this);
    if (is_main_thread)
        native_thread_->init_main();
    else
        native_thread_->create();
}

ThreadContext::~ThreadContext() {}

void ThreadContext::enter(interpreter::PyFrame* frame)
{
    frame_stack_.push_back(frame);
}

void ThreadContext::leave(interpreter::PyFrame* frame)
{
    frame_stack_.pop_back();
}

void ThreadContext::bind_gc(gc::GarbageCollector* gc)
{
    gc_context_ = gc->create_context(this);
    gc_ = gc;
}

void ThreadContext::save_machine_context()
{
    setjmp(machine_regs_);
    native_thread_->save_stack_end();
}

void ThreadContext::mark_roots(gc::GarbageCollector* gc)
{
    for (const auto& frame : frame_stack_)
        gc->mark_object(frame);

    auto& stack = native_thread_->get_stack();
    M_BaseObject** objp = (M_BaseObject**)stack.stack_end;

    while (objp < stack.stack_start) {
        gc->mark_object_maybe(*objp);
        objp++;
    }

    objp = (M_BaseObject**)&machine_regs_;
    while (objp <
           (M_BaseObject**)((char*)&machine_regs_ + sizeof(machine_regs_))) {
        gc->mark_object_maybe(*objp);
        objp++;
    }
}
