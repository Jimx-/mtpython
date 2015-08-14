#ifndef _INTERPRETER_GATEWAY_H_
#define _INTERPRETER_GATEWAY_H_

#include "interpreter/code.h"
#include "interpreter/arguments.h"
#include "objects/base_object.h"
#include "objects/obj_space.h"
#include "vm/vm.h"
#include "macros.h"
#include <string>

namespace mtpython {
namespace interpreter {

/* Wrapper of interpreter level function */
typedef mtpython::objects::M_BaseObject* (*InterpFunction)(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self, Arguments& args);
typedef mtpython::objects::M_BaseObject* (*InterpFunction1)(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* arg1);
typedef mtpython::objects::M_BaseObject* (*InterpFunction2)(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* arg1, mtpython::objects::M_BaseObject* arg2);


class BuiltinCode : public Code {
private:
	InterpFunction func;
public:
	BuiltinCode(InterpFunction f) : Code(std::string("<builtin func>")) { func = f; }

	virtual mtpython::objects::M_BaseObject* funcrun(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, Arguments& args)
	{
		return funcrun_obj(context, func, nullptr, args);
	}

	virtual mtpython::objects::M_BaseObject* funcrun_obj(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, mtpython::objects::M_BaseObject* obj, Arguments& args);
};

class BuiltinCode1 : public Code {
private:
	InterpFunction1 func;
public:
	BuiltinCode1(InterpFunction1 f) : Code(std::string("<builtin func with 1 args>")) { func = f; }

	virtual mtpython::objects::M_BaseObject* funcrun(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, Arguments& args);
	virtual mtpython::objects::M_BaseObject* funcrun_obj(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, mtpython::objects::M_BaseObject* obj, Arguments& args)
	{
		args.prepend(obj);
		return funcrun(context, func, args);
	}

};

class BuiltinCode2 : public Code {
private:
	InterpFunction2 func;
public:
	BuiltinCode2(InterpFunction2 f) : Code(std::string("<builtin func with 2 args>")) { func = f; }

	virtual mtpython::objects::M_BaseObject* funcrun(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, Arguments& args);
	virtual mtpython::objects::M_BaseObject* funcrun_obj(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, mtpython::objects::M_BaseObject* obj, Arguments& args)
	{
		args.prepend(obj);
		return funcrun(context, func, args);
	}

};

class InterpFunctionWrapper : public mtpython::objects::M_BaseObject {
protected:
	Code* code;
public:
	InterpFunctionWrapper(InterpFunction f);
	InterpFunctionWrapper(InterpFunction1 f);
	InterpFunctionWrapper(InterpFunction2 f);
	~InterpFunctionWrapper() { SAFE_DELETE(code); }

	virtual M_BaseObject* bind_space(objects::ObjSpace* space) { return space->get_gateway_cache(this); }

	Code* get_code() { return code; }
};

}
}

#endif /* _INTERPRETER_GATEWAY_H_ */
