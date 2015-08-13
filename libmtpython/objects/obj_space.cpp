#include "vm/vm.h"
#include "interpreter/compiler.h"
#include "objects/obj_space.h"

using namespace mtpython::objects;
using namespace mtpython::vm;
using namespace mtpython::interpreter;

BaseCompiler* ObjSpace::get_compiler(ThreadContext* context)
{
	return new PyCompiler(context);
}
