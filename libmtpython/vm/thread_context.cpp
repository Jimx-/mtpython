#include "vm/vm.h"

using namespace mtpython::vm;

ThreadContext::ThreadContext(PyVM* vm, mtpython::objects::ObjSpace* space)
{
	this->vm = vm;
	this->space = space;
}
