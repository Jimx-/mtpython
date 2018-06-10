#ifndef _VM_VM_H_
#define _VM_VM_H_

#include "objects/obj_space.h"
#include "gc/heap.h"

#include <stack>
#include <vector>
#include <unordered_set>
#include <functional>

namespace mtpython {

namespace interpreter {
class BaseCompiler;
class PyFrame;
class Module;
}

namespace vm {

class PyVM;

class LocalFrame {
private:
	std::unordered_set<objects::M_BaseObject*> local_refs;
public:

	void new_local_ref(objects::M_BaseObject* obj);
	void delete_local_ref(objects::M_BaseObject* obj);
};

class ThreadContext {
private:
	PyVM* vm;
	objects::ObjSpace* space;
	interpreter::BaseCompiler* compiler;

	std::stack<interpreter::PyFrame*> frame_stack;
	std::vector<LocalFrame*> local_frame_stack;
	LocalFrame* top_local_frame;
public:
	ThreadContext(PyVM* vm, objects::ObjSpace* space);
	~ThreadContext();

	interpreter::BaseCompiler* get_compiler() { return compiler; }
	objects::ObjSpace* get_space() { return space; }
	gc::Heap* heap();

	void enter(interpreter::PyFrame* frame);
	void leave(interpreter::PyFrame* frame);
	interpreter::PyFrame* top_frame() { return frame_stack.top(); }

	void push_local_frame();
	objects::M_BaseObject* pop_local_frame(objects::M_BaseObject* result);

	void new_local_ref(objects::M_BaseObject* obj);
	void delete_local_ref(objects::M_BaseObject* obj);

	void acquire_import_lock() { }
	void release_import_lock() { }
};

class PyVM {
private:
	objects::ObjSpace* space;
	gc::Heap* heap;

	ThreadContext main_thread;

	mtpython::interpreter::Module* init_main_module(ThreadContext* context);
	mtpython::interpreter::Code* compile_code(ThreadContext* context, const std::string& source,
				const std::string& filename, const std::string& mode);
	void run_eval_string(ThreadContext* context, const std::string &source,
							   const std::string &filename, bool eval);
public:
	PyVM(mtpython::objects::ObjSpace* space, const std::string& executable);

	/* we have only one thread now */
	ThreadContext* current_thread() { return &main_thread; }
	gc::Heap* get_heap() { return heap; }

	void init_bootstrap_path(const std::string& executable);
	void run_file(const std::string& filename);
	void run_toplevel(std::function<void()> f);
};

inline gc::Heap* ThreadContext::heap()
{
	if (vm)
		return vm->get_heap();

	return nullptr;
}

}
}

#endif /* _VM_VM_H_ */
