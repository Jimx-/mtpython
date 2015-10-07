#include <assert.h>
#include "vm/vm.h"
#include "interpreter/compiler.h"
#include "macros.h"

using namespace mtpython::vm;
using namespace mtpython::objects;

void LocalFrame::new_local_ref(M_BaseObject* obj)
{
	local_refs.insert(obj);
}

void LocalFrame::delete_local_ref(M_BaseObject* obj)
{
	local_refs.erase(obj);
}

ThreadContext::ThreadContext(PyVM* vm, ObjSpace* space)
{
	this->vm = vm;
	this->space = space;
	this->compiler = space->get_compiler(this);

	push_local_frame();
}

ThreadContext::~ThreadContext()
{
	SAFE_DELETE(compiler);
}

void ThreadContext::enter(mtpython::interpreter::PyFrame* frame)
{
	frame_stack.push(frame);
}

void ThreadContext::leave(mtpython::interpreter::PyFrame* frame)
{
	frame_stack.pop();
}

M_BaseObject* ThreadContext::new_object(M_BaseObject* obj)
{
	new_local_ref(obj);

	return obj;
}

void ThreadContext::new_local_ref(M_BaseObject* obj)
{
	top_local_frame->new_local_ref(obj);
}

void ThreadContext::delete_local_ref(M_BaseObject* obj)
{
	top_local_frame->delete_local_ref(obj);
}

void ThreadContext::push_local_frame()
{
	LocalFrame* frame = new LocalFrame();
	top_local_frame = frame;
	local_frame_stack.push_back(frame);
}

M_BaseObject* ThreadContext::pop_local_frame(M_BaseObject* result)
{
	assert(local_frame_stack.size() > 1);

	LocalFrame* outer_frame = local_frame_stack[local_frame_stack.size() - 2];
	outer_frame->new_local_ref(result);

	local_frame_stack.pop_back();
	top_local_frame = local_frame_stack.back();

	return result;
}

