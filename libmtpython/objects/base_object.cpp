#include "objects/base_object.h"
#include "objects/obj_space.h"

using namespace mtpython::objects;

M_BaseObject* M_BaseObject::get_class(ObjSpace* space)
{
	return space->get_typeobject(get_typedef());
}
