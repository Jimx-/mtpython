#ifndef _INTERPRETER_MODULE_H_
#define _INTERPRETER_MODULE_H_

#include "objects/base_object.h"
#include "objects/obj_space.h"
#include "interpreter/code.h"
#include "interpreter/arguments.h"
#include "interpreter/typedef.h"
#include "vm/vm.h"
#include <string>

namespace mtpython {
namespace interpreter {

class Module : public objects::M_BaseObject {
protected:
	objects::ObjSpace* space;
	objects::M_BaseObject* name;
	objects::M_BaseObject* dict;

	void add_def(const std::string& name, objects::M_BaseObject* value)
	{
		space->setitem(dict, space->new_interned_str(name), space->wrap(space->current_thread(), value));
	}
public:
	Module(objects::ObjSpace* space, objects::M_BaseObject* name, objects::M_BaseObject* dict=nullptr);

	Typedef* get_typedef();

	objects::M_BaseObject* get_dict(objects::ObjSpace* space) { return dict; }
	objects::M_BaseObject* get(const std::string& name);
	objects::M_BaseObject* call(vm::ThreadContext* context, const std::string& name, const std::initializer_list<objects::M_BaseObject*> args);
	objects::M_BaseObject* get_dict_value(objects::ObjSpace* space, const std::string& attr);
	
	virtual void install();

	static objects::M_BaseObject* __repr__(vm::ThreadContext* context, objects::M_BaseObject* self);
	static objects::M_BaseObject* __dict__get(vm::ThreadContext* context, objects::M_BaseObject* self);
};

class BuiltinModule : public Module {
public:
	BuiltinModule(objects::ObjSpace* space, objects::M_BaseObject* name, objects::M_BaseObject* dict=nullptr) : Module(space, name, dict) { }
};

}
}

#endif /* _INTERPRETER_MODULE_H_ */
