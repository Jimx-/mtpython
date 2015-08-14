#include "interpreter/gateway.h"
#include "interpreter/function.h"
#include "objects/space_cache.h"
#include "exceptions.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;
using namespace mtpython::vm;

M_BaseObject* BuiltinCode::funcrun_obj(ThreadContext* context, M_BaseObject* func, M_BaseObject* obj, Arguments& args)
{
	return this->func(context, obj, args);
}

M_BaseObject* BuiltinCode1::funcrun(ThreadContext* context, M_BaseObject* func, Arguments& args)
{
	if (args.size() != 1) throw TypeError("function takes exactly 1 arguments");

	return this->func(context, args[0]);
}

M_BaseObject* BuiltinCode2::funcrun(ThreadContext* context, M_BaseObject* func, Arguments& args)
{
	if (args.size() != 2) throw TypeError("function takes exactly 2 arguments");

	return this->func(context, args[0], args[1]);
}

InterpFunctionWrapper::InterpFunctionWrapper(InterpFunction f)
{
	code = new BuiltinCode(f);
}

InterpFunctionWrapper::InterpFunctionWrapper(InterpFunction1 f)
{
	code = new BuiltinCode1(f);
}

InterpFunctionWrapper::InterpFunctionWrapper(InterpFunction2 f)
{
	code = new BuiltinCode2(f);
}

M_BaseObject* GatewayCache::build(M_BaseObject* key)
{
	InterpFunctionWrapper* wrapper = dynamic_cast<InterpFunctionWrapper*>(key);
	if (!wrapper) return nullptr;

	Function* f = new Function(space, wrapper->get_code());

	return f;
}
