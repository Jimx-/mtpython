#include <string.h>
#include "objects/base_object.h"
#include "objects/std/obj_space_std.h"
#include "objects/std/type_object.h"
#include "objects/std/int_object.h"
#include "objects/std/object_object.h"

using namespace mtpython::objects;

StdObjSpace::StdObjSpace() : ObjSpace()
{
	typedef_cache = new StdTypedefCache(this);

	builtin_types["int"] = get_typeobject(M_StdIntObject::_int_typedef());
	builtin_types["object"] = get_typeobject(M_StdObjectObject::_object_typedef());
	builtin_types["type"] = get_typeobject(StdTypeObject::_type_typedef());
}

M_BaseObject* StdObjSpace::wrap_int(int x)
{
	return new M_StdIntObject(x);
}

M_BaseObject* StdObjSpace::wrap_int(std::string& x)
{
	return new M_StdIntObject(x);
}
