#ifndef _VM_VM_H_
#define _VM_VM_H_

#include "objects/obj_space.h"

namespace mtpython {

namespace interpreter {
class BaseCompiler;
}

namespace vm {

class PyVM;

class ThreadContext {
private:
	PyVM* vm;
	mtpython::objects::ObjSpace* space;
	mtpython::interpreter::BaseCompiler* compiler;
public:
	ThreadContext(PyVM* vm, mtpython::objects::ObjSpace* space);
	~ThreadContext();

	mtpython::interpreter::BaseCompiler* get_compiler() { return compiler; }
	mtpython::objects::ObjSpace* get_space() { return space; }

};

class PyVM {
private:
	mtpython::objects::ObjSpace* space;

	ThreadContext main_thread;
public:
	PyVM(mtpython::objects::ObjSpace* space);

	void run_file(std::string& filename);
};

}
}

#endif /* _VM_VM_H_ */
