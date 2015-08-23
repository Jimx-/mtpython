#include "interpreter/gateway.h"
#include "interpreter/function.h"
#include "objects/space_cache.h"
#include "exceptions.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;
using namespace mtpython::vm;

M_BaseObject* BuiltinCode::funcrun_obj(ThreadContext* context, M_BaseObject* func, M_BaseObject* obj, Arguments& args)
{
	std::vector<M_BaseObject*> scope;
	args.parse(obj, sig, scope);

	return this->func(context, scope);
}

M_BaseObject* BuiltinCode1::funcrun_obj(ThreadContext* context, M_BaseObject* func, M_BaseObject* obj, Arguments& args)
{
	std::vector<M_BaseObject*> scope;
	args.parse(obj, sig, scope);

	return this->func(context, scope[0]);
}

M_BaseObject* BuiltinCode2::funcrun_obj(ThreadContext* context, M_BaseObject* func, M_BaseObject* obj, Arguments& args)
{
	std::vector<M_BaseObject*> scope;
	args.parse(obj, sig, scope);

	return this->func(context, scope[0], scope[1]);
}

M_BaseObject* BuiltinCode3::funcrun_obj(ThreadContext* context, M_BaseObject* func, M_BaseObject* obj, Arguments& args)
{
	std::vector<M_BaseObject*> scope;
	args.parse(obj, sig, scope);

	return this->func(context, scope[0], scope[1], scope[2]);
}

InterpFunctionWrapper::InterpFunctionWrapper(InterpFunction f, Signature& sig)
{
	code = new BuiltinCode(f, sig);
}

InterpFunctionWrapper::InterpFunctionWrapper(InterpFunction1 f)
{
	code = new BuiltinCode1(f);
}

InterpFunctionWrapper::InterpFunctionWrapper(InterpFunction1 f, Signature& sig)
{
	code = new BuiltinCode1(f, sig);
}

InterpFunctionWrapper::InterpFunctionWrapper(InterpFunction2 f)
{
	code = new BuiltinCode2(f);
}

InterpFunctionWrapper::InterpFunctionWrapper(InterpFunction2 f, Signature& sig)
{
	code = new BuiltinCode2(f, sig);
}

InterpFunctionWrapper::InterpFunctionWrapper(InterpFunction3 f)
{
	code = new BuiltinCode3(f);
}

InterpFunctionWrapper::InterpFunctionWrapper(InterpFunction3 f, Signature& sig)
{
	code = new BuiltinCode3(f, sig);
}

M_BaseObject* GatewayCache::build(M_BaseObject* key)
{
	InterpFunctionWrapper* wrapper = dynamic_cast<InterpFunctionWrapper*>(key);
	if (!wrapper) return nullptr;

	Function* f = new Function(space, wrapper->get_code());

	return f;
}
