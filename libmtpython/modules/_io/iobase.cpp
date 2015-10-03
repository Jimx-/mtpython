#include "modules/_io/iomodule.h"
#include "modules/_io/iobase.h"
#include "interpreter/gateway.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"
#include "interpreter/error.h"
#include "interpreter/pyframe.h"

using namespace mtpython::modules;
using namespace mtpython::objects;
using namespace mtpython::interpreter;

M_IOBase::M_IOBase(ObjSpace* space)
{
	dict = space->new_dict();
}
