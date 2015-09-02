#include "objects/base_object.h"
#include "objects/obj_space.h"

using namespace mtpython::objects;

M_BaseObject* M_BaseObject::get_class(ObjSpace* space)
{
	return space->get_typeobject(get_typedef());
}

M_BaseObject* M_BaseObject::get_dict_value(ObjSpace* space, const std::string& attr)
{
	M_BaseObject* dict = get_dict(space);
	if (dict) {
		return space->getitem_str(dict, attr);
	}
	return nullptr;
}

bool M_BaseObject::set_dict_value(ObjSpace* space, const std::string& attr, M_BaseObject* value)
{
	M_BaseObject* dict = get_dict(space);

	if (dict) {
		space->setitem_str(dict, attr, value);
		return true;
	}

	return false;
}
