#include <assert.h>
#include "vm/vm.h"
#include "interpreter/compiler.h"
#include "macros.h"

#include "vm/native_thread_posix.h"

using namespace mtpython::vm;
using namespace mtpython::objects;
using namespace mtpython::gc;

void ThreadContext::LocalFrame::new_local_ref(M_BaseObject* obj)
{
    local_refs.insert(obj);
}

void ThreadContext::LocalFrame::delete_local_ref(M_BaseObject* obj)
{
    local_refs.erase(obj);
}

ThreadContext::ThreadContext(PyVM* vm, ObjSpace* space, bool is_main_thread)
    : vm_(vm), is_main_thread_(is_main_thread), space_(space)
{
    compiler_ = space->get_compiler(this);

    native_thread_ = std::make_unique<NativeThreadPOSIX>(this);
    if (is_main_thread)
        native_thread_->init_main();
    else
        native_thread_->create();

    push_local_frame();
}

ThreadContext::~ThreadContext() { SAFE_DELETE(compiler_); }

void ThreadContext::enter(mtpython::interpreter::PyFrame* frame)
{
    frame_stack_.push(frame);
}

void ThreadContext::leave(mtpython::interpreter::PyFrame* frame)
{
    frame_stack_.pop();
}

void ThreadContext::new_local_ref(M_BaseObject* obj)
{
    top_local_frame_->new_local_ref(obj);
}

void ThreadContext::delete_local_ref(M_BaseObject* obj)
{
    top_local_frame_->delete_local_ref(obj);
}

void ThreadContext::push_local_frame()
{
    LocalFrame* frame = new LocalFrame();
    top_local_frame_ = frame;
    local_frame_stack_.push_back(frame);
}

M_BaseObject* ThreadContext::pop_local_frame(M_BaseObject* result)
{
    assert(local_frame_stack_.size() > 1);

    if (result) {
        LocalFrame* outer_frame =
            local_frame_stack_[local_frame_stack_.size() - 2];
        outer_frame->new_local_ref(result);
    }

    LocalFrame* top = local_frame_stack_.back();
    local_frame_stack_.pop_back();
    SAFE_DELETE(top);
    top_local_frame_ = local_frame_stack_.back();

    return result;
}
