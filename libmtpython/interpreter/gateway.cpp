#include "interpreter/gateway.h"
#include "interpreter/function.h"
#include "interpreter/error.h"
#include "objects/space_cache.h"
#include "exceptions.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;
using namespace mtpython::vm;

M_BaseObject* BuiltinCode::funcrun_obj(ThreadContext* context, M_BaseObject* func, M_BaseObject* obj, Arguments& args)
{
	ObjSpace* space = context->get_space();
	Function* as_func = dynamic_cast<Function*>(func);
	if (!as_func)
		throw InterpError(space->TypeError_type(), space->wrap_str("expected Functon object"));

	std::vector<M_BaseObject*> scope;
	args.parse(as_func->get_name(), obj, sig, scope);

	return this->func(context, scope);
}

M_BaseObject* BuiltinCode1::funcrun_obj(ThreadContext* context, M_BaseObject* func, M_BaseObject* obj, Arguments& args)
{
	ObjSpace* space = context->get_space();
	Function* as_func = dynamic_cast<Function*>(func);
	if (!as_func)
		throw InterpError(space->TypeError_type(), space->wrap_str("expected Functon object"));

	std::vector<M_BaseObject*> scope;
	args.parse(as_func->get_name(), obj, sig, scope);

	return this->func(context, scope[0]);
}

M_BaseObject* BuiltinCode2::funcrun_obj(ThreadContext* context, M_BaseObject* func, M_BaseObject* obj, Arguments& args)
{
	ObjSpace* space = context->get_space();
	Function* as_func = dynamic_cast<Function*>(func);
	if (!as_func)
		throw InterpError(space->TypeError_type(), space->wrap_str("expected Functon object"));

	std::vector<M_BaseObject*> scope;
	args.parse(as_func->get_name(), obj, sig, scope);

	return this->func(context, scope[0], scope[1]);
}

M_BaseObject* BuiltinCode3::funcrun_obj(ThreadContext* context, M_BaseObject* func, M_BaseObject* obj, Arguments& args)
{
	ObjSpace* space = context->get_space();
	Function* as_func = dynamic_cast<Function*>(func);
	if (!as_func)
		throw InterpError(space->TypeError_type(), space->wrap_str("expected Functon object"));

	std::vector<M_BaseObject*> scope;
	args.parse(as_func->get_name(), obj, sig, scope);

	return this->func(context, scope[0], scope[1], scope[2]);
}

InterpFunctionWrapper::InterpFunctionWrapper(const std::string& name, InterpFunction f, Signature& sig)
{
	code = new BuiltinCode(name, f, sig);
}

InterpFunctionWrapper::InterpFunctionWrapper(const std::string& name, InterpFunction1 f)
{
	code = new BuiltinCode1(name, f);
}

InterpFunctionWrapper::InterpFunctionWrapper(const std::string& name, InterpFunction1 f, Signature& sig)
{
	code = new BuiltinCode1(name, f, sig);
}

InterpFunctionWrapper::InterpFunctionWrapper(const std::string& name, InterpFunction2 f)
{
	code = new BuiltinCode2(name, f);
}

InterpFunctionWrapper::InterpFunctionWrapper(const std::string& name, InterpFunction2 f, Signature& sig)
{
	code = new BuiltinCode2(name, f, sig);
}

InterpFunctionWrapper::InterpFunctionWrapper(const std::string& name, InterpFunction3 f)
{
	code = new BuiltinCode3(name, f);
}

InterpFunctionWrapper::InterpFunctionWrapper(const std::string& name, InterpFunction3 f, Signature& sig)
{
	code = new BuiltinCode3(name, f, sig);
}

M_BaseObject* GatewayCache::build(M_BaseObject* key)
{
	InterpFunctionWrapper* wrapper = dynamic_cast<InterpFunctionWrapper*>(key);
	if (!wrapper) return nullptr;

	Function* f = new Function(space, wrapper->get_code());

	return f;
}
