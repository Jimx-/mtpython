#include "objects/base_object.h"
#include "objects/obj_space.h"
#include "macros.h"

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

bool M_BaseObject::del_dict_value(ObjSpace* space, const std::string& attr)
{
	M_BaseObject* dict = get_dict(space);

	if (dict) {
		M_BaseObject* wrapped_attr = space->wrap_str(attr);
		space->delitem(dict, wrapped_attr);
		SAFE_DELETE(wrapped_attr);
		return true;
	}

	return false;
}

M_BaseObject* M_BaseObject::get_repr(ObjSpace* space, const std::string& info)
{
	std::string repr_string = "<" + info + ">";
	return space->wrap_str(repr_string);
}
