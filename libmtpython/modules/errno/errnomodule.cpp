#include "modules/errno/errnomodule.h"
#include "interpreter/gateway.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"
#include "interpreter/error.h"
#include "interpreter/pyframe.h"

using namespace mtpython::modules;
using namespace mtpython::objects;
using namespace mtpython::interpreter;

ErrnoModule::ErrnoModule(ObjSpace* space, M_BaseObject* name)
    : BuiltinModule(space, name)
{
    vm::ThreadContext* context = vm::ThreadContext::current_thread();
#include "tools/errno_def.h"
}
