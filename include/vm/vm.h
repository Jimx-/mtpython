#ifndef _VM_VM_H_
#define _VM_VM_H_

#include <stack>

#include "objects/obj_space.h"

namespace mtpython {

namespace interpreter {
class BaseCompiler;
class PyFrame;
class Module;
}

namespace vm {

class PyVM;

class ThreadContext {
private:
	PyVM* vm;
	mtpython::objects::ObjSpace* space;
	mtpython::interpreter::BaseCompiler* compiler;

	std::stack<mtpython::interpreter::PyFrame*> frame_stack;
public:
	ThreadContext(PyVM* vm, mtpython::objects::ObjSpace* space);
	~ThreadContext();

	mtpython::interpreter::BaseCompiler* get_compiler() { return compiler; }
	mtpython::objects::ObjSpace* get_space() { return space; }

	void enter(mtpython::interpreter::PyFrame* frame);
	void leave(mtpython::interpreter::PyFrame* frame);
	mtpython::interpreter::PyFrame* top_frame() { return frame_stack.top(); }

	void gc_track_object(mtpython::objects::M_BaseObject* obj) { }
	void gc_untrack_object(mtpython::objects::M_BaseObject* obj) { }
};

class PyVM {
private:
	mtpython::objects::ObjSpace* space;

	ThreadContext main_thread;

	mtpython::interpreter::Module* init_main_module(ThreadContext* context);
	mtpython::interpreter::Code* compile_code(ThreadContext* context, const std::string& source,
				const std::string& filename, const std::string& mode);
	void run_eval_string(ThreadContext* context, const std::string &source,
							   const std::string &filename, bool eval);
public:
	PyVM(mtpython::objects::ObjSpace* space);

	/* we have only one thread now */
	ThreadContext* current_thread() { return &main_thread; }

	void run_file(std::string& filename);
};

}
}

#endif /* _VM_VM_H_ */
