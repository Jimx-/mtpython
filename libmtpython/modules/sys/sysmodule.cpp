#include "modules/sys/sysmodule.h"
#include "interpreter/gateway.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"
#include "interpreter/error.h"
#include "interpreter/pyframe.h"

using namespace mtpython::modules;
using namespace mtpython::objects;
using namespace mtpython::interpreter;

SysModule::SysModule(ObjSpace* space, M_BaseObject* name) : BuiltinModule(space, name)
{
    add_def("modules", space->new_dict());
}
