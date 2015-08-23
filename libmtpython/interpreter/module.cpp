#include "interpreter/module.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;

Module::Module(ObjSpace* space, M_BaseObject* name, M_BaseObject* dict)
{
	this->space = space;
	this->name = name;

	this->dict = (dict != nullptr) ? dict : space->new_dict();

	if (name) {
		space->setitem(this->dict, space->new_interned_str(std::string("__name__")), name);
	}
}


void Module::install()
{
	std::string s_name = space->unwrap_str(name);
	M_BaseObject* wrapped = space->wrap(this);
	space->set_builtin_module(s_name, wrapped);
}

M_BaseObject* Module::get_dict_value(ObjSpace* space, std::string& attr)
{
	return space->getitem_str(dict, attr);
}
