#ifndef _INTERPRETER_GATEWAY_H_
#define _INTERPRETER_GATEWAY_H_

#include "interpreter/code.h"
#include "interpreter/arguments.h"
#include "interpreter/signature.h"
#include "objects/base_object.h"
#include "objects/obj_space.h"
#include "vm/vm.h"
#include "macros.h"
#include <string>
#include <vector>
#include <initializer_list>

namespace mtpython {
namespace interpreter {

/* Wrapper of interpreter level function */
typedef mtpython::objects::M_BaseObject* (*InterpFunction)(mtpython::vm::ThreadContext* context, std::vector<mtpython::objects::M_BaseObject*>& args);
typedef mtpython::objects::M_BaseObject* (*InterpFunction1)(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* arg1);
typedef mtpython::objects::M_BaseObject* (*InterpFunction2)(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* arg1, mtpython::objects::M_BaseObject* arg2);
typedef mtpython::objects::M_BaseObject* (*InterpFunction3)(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* arg1, mtpython::objects::M_BaseObject* arg2, mtpython::objects::M_BaseObject* arg3);

class BuiltinCode : public Code {
private:
	InterpFunction func;
	Signature sig;
public:
	BuiltinCode(const std::string& name, InterpFunction f, const Signature& sig) : Code(name), sig(sig) { func = f; }

	virtual mtpython::objects::M_BaseObject* funcrun(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, Arguments& args)
	{
		return funcrun_obj(context, func, nullptr, args);
	}

	virtual mtpython::objects::M_BaseObject* funcrun_obj(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, mtpython::objects::M_BaseObject* obj, Arguments& args);
};

class BuiltinCode1 : public Code {
private:
	InterpFunction1 func;
	Signature sig;
public:
	BuiltinCode1(const std::string& name, InterpFunction1 f) : Code(name), sig({"arg0"}) { func = f; }
	BuiltinCode1(const std::string& name, InterpFunction1 f, const Signature& sig) : Code(name), sig(sig) { func = f; }

	virtual mtpython::objects::M_BaseObject* funcrun(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, Arguments& args)
	{
		return funcrun_obj(context, func, nullptr, args);
	}
	virtual mtpython::objects::M_BaseObject* funcrun_obj(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, mtpython::objects::M_BaseObject* obj, Arguments& args);

};

class BuiltinCode2 : public Code {
private:
	InterpFunction2 func;
	Signature sig;
public:
	BuiltinCode2(const std::string& name, InterpFunction2 f) : Code(name), sig({"arg0", "arg1"}) { func = f; }
	BuiltinCode2(const std::string& name, InterpFunction2 f, const Signature& sig) : Code(name), sig(sig) { func = f; }

	virtual mtpython::objects::M_BaseObject* funcrun(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, Arguments& args)
	{
		return funcrun_obj(context, func, nullptr, args);
	}
	virtual mtpython::objects::M_BaseObject* funcrun_obj(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, mtpython::objects::M_BaseObject* obj, Arguments& args);

};

class BuiltinCode3 : public Code {
private:
	InterpFunction3 func;
	Signature sig;
public:
	BuiltinCode3(const std::string& name, InterpFunction3 f) : Code(name), sig({"arg0", "arg1", "arg2"}) { func = f; }
	BuiltinCode3(const std::string& name, InterpFunction3 f, const Signature& sig) : Code(name), sig(sig) { func = f; }

	virtual mtpython::objects::M_BaseObject* funcrun(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, Arguments& args)
	{
		return funcrun_obj(context, func, nullptr, args);
	}
	virtual mtpython::objects::M_BaseObject* funcrun_obj(vm::ThreadContext* context, mtpython::objects::M_BaseObject* func, mtpython::objects::M_BaseObject* obj, Arguments& args);

};

class InterpFunctionWrapper : public mtpython::objects::M_BaseObject {
protected:
	Code* code;
public:
	InterpFunctionWrapper(const std::string& name, InterpFunction f, const Signature& sig);
	InterpFunctionWrapper(const std::string& name, InterpFunction1 f);
	InterpFunctionWrapper(const std::string& name, InterpFunction1 f, const Signature& sig);
	InterpFunctionWrapper(const std::string& name, InterpFunction2 f);
	InterpFunctionWrapper(const std::string& name, InterpFunction2 f, const Signature& sig);
	InterpFunctionWrapper(const std::string& name, InterpFunction3 f);
	InterpFunctionWrapper(const std::string& name, InterpFunction3 f, const Signature& sig);
	~InterpFunctionWrapper() { SAFE_DELETE(code); }

	virtual M_BaseObject* bind_space(objects::ObjSpace* space) { return space->get_gateway_cache(this); }

	Code* get_code() { return code; }
};

}
}

#endif /* _INTERPRETER_GATEWAY_H_ */
