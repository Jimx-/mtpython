#include "modules/_weakref/weakrefmodule.h"
#include "interpreter/gateway.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"
#include "interpreter/error.h"
#include "interpreter/pyframe.h"

using namespace mtpython::modules;
using namespace mtpython::objects;
using namespace mtpython::interpreter;

WeakrefModule::WeakrefModule(mtpython::objects::ObjSpace* space, M_BaseObject* name) : BuiltinModule(space, name)
{
    add_def("ref", space->wrap_None());
}

