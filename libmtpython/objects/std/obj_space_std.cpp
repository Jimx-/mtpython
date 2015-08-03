#include <string.h>
#include "objects/base_object.h"
#include "objects/std/obj_space_std.h"
#include "objects/std/int_object.h"

using namespace mtpython::objects;

M_BaseObject* StdObjSpace::wrap_int(int x)
{
	return new M_StdIntObject(x);
}

M_BaseObject* StdObjSpace::wrap_int(std::string& x)
{
	return new M_StdIntObject(x);
}
