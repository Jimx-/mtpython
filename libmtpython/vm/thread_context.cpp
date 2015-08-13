#include "vm/vm.h"
#include "interpreter/compiler.h"
#include "macros.h"

using namespace mtpython::vm;

ThreadContext::ThreadContext(PyVM* vm, mtpython::objects::ObjSpace* space)
{
	this->vm = vm;
	this->space = space;
	this->compiler = space->get_compiler(this);
}

ThreadContext::~ThreadContext()
{
	SAFE_DELETE(compiler);
}
