#include "interpreter/module.h"
#include "interpreter/error.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;

Module::Module(ObjSpace* space, M_BaseObject* name, M_BaseObject* dict)
{
	this->space = space;
	this->name = name;

	this->dict = (dict != nullptr) ? dict : space->new_dict();

	if (name) {
		space->setitem(this->dict, space->new_interned_str("__name__"), name);
	}
}

void Module::install()
{
	std::string s_name = space->unwrap_str(name);
	M_BaseObject* wrapped = space->wrap(this);
	space->set_builtin_module(s_name, wrapped);
}

M_BaseObject* Module::get_dict_value(ObjSpace* space, const std::string& attr)
{
	return space->getitem_str(dict, attr);
}

M_BaseObject* Module::get(const std::string &name)
{
	M_BaseObject* value = get_dict_value(space, name);
	if (!value) throw InterpError(space->TypeError_type(), space->wrap(name));
	return value;
}

M_BaseObject* Module::call(mtpython::vm::ThreadContext* context, const std::string &name,
						   const std::initializer_list<M_BaseObject*> args)
{
	M_BaseObject* func = get(name);
	return space->call_function(context, func, args);
}
