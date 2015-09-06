#include "interpreter/module.h"
#include "interpreter/error.h"
#include "interpreter/gateway.h"
#include "interpreter/descriptor.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;

static Typedef Module_typedef("module", {
	{ "__repr__", new InterpFunctionWrapper("__repr__", Module::__repr__) },
	{ "__dict__", new GetSetDescriptor(Module::__dict__get )},
});

Module::Module(ObjSpace* space, M_BaseObject* name, M_BaseObject* dict)
{
	this->space = space;
	this->name = name;

	this->dict = (dict != nullptr) ? dict : space->new_dict();

	if (name) {
		space->setitem(this->dict, space->new_interned_str("__name__"), name);
	}
}

Typedef* Module::get_typedef()
{
	return &Module_typedef;
}

void Module::install()
{
	std::string s_name = space->unwrap_str(name);
	M_BaseObject* wrapped = space->wrap(this);
	space->set_builtin_module(s_name, wrapped);
}

M_BaseObject* Module::get_dict_value(ObjSpace* space, const std::string& attr)
{
	return space->finditem_str(dict, attr);
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

M_BaseObject* Module::__repr__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	Module* as_mod = static_cast<Module*>(self);
	std::string str;
	ObjSpace* space = context->get_space();
	M_BaseObject* repr_str = space->repr(as_mod->name);
	str += "<module " + space->unwrap_str(repr_str);
	SAFE_DELETE(repr_str);

	if (dynamic_cast<BuiltinModule*>(as_mod)) {
		str += " (built-in)>";
	} else {
		M_BaseObject* attr = space->wrap_str("__file__");
		M_BaseObject* wrapped_file = space->getattr(self, attr);
		context->gc_track_object(attr);
		std::string filename = space->unwrap_str(wrapped_file);
		str += " from '" + filename + "'>";
	}

	return space->wrap_str(str);
}

M_BaseObject* Module::__dict__get(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	Module* mod = static_cast<Module*>(self);
	return mod->dict;
}
