#include "vm/vm.h"
#include "interpreter/compiler.h"
#include "interpreter/function.h"
#include "interpreter/error.h"
#include "objects/obj_space.h"

#include "modules/builtins/bltinmodule.h"

#include "macros.h"
#include "exceptions.h"

using namespace mtpython::objects;
using namespace mtpython::vm;
using namespace mtpython::interpreter;

ObjSpace::ObjSpace() : gateway_cache(this) 
{
	vm = nullptr;
	dummy_context = new ThreadContext(nullptr, this);
}

ObjSpace::~ObjSpace()
{
	SAFE_DELETE(dummy_context);
}

ThreadContext* ObjSpace::current_thread()
{
	if (vm) return vm->current_thread();
	return dummy_context;
}

void ObjSpace::make_builtins()
{
	M_BaseObject* builtins_name;
	builtins_name = wrap(std::string("builtins"));
	mtpython::modules::BuiltinsModule* builtins_mod = new mtpython::modules::BuiltinsModule(this, builtins_name);
	builtins_mod->install();
	builtin = builtins_mod;
	setitem(builtins_mod->get_dict(), wrap(std::string("__builtins__")), wrap(builtins_mod));

	init_builtin_exceptions();
}

void ObjSpace::init_builtin_exceptions()
{
#define SET_EXCEPTION_TYPE(name) type_##name = builtin->get_dict_value(this, std::string(#name));
	SET_EXCEPTION_TYPE(TypeError); 
}

BaseCompiler* ObjSpace::get_compiler(ThreadContext* context)
{
	return new PyCompiler(context);
}

M_BaseObject* ObjSpace::execute_binop(M_BaseObject* impl, M_BaseObject* left, M_BaseObject* right)
{
	Function* func = dynamic_cast<Function*>(impl);
	if (!func) return nullptr;

	return call_function(current_thread(), func, {left, right});
}	

M_BaseObject* ObjSpace::hash(M_BaseObject* obj)
{
	M_BaseObject* descr = lookup(obj, std::string("__hash__"));
	if (!descr) {
		return wrap((int)obj);
	}

	M_BaseObject* hash_value = get_and_call_function(current_thread(), descr, {obj});

	return hash_value;
}

bool ObjSpace::is_true(M_BaseObject* obj)
{
	if (i_is(obj, wrap_True())) return true;
	if (i_is(obj, wrap_False())) return false;

	return false;
}

bool ObjSpace::i_eq(M_BaseObject* obj1, M_BaseObject* obj2)
{
	return i_is(obj1, obj2) && is_true(eq(obj1, obj2));
}

std::size_t ObjSpace::i_hash(M_BaseObject* obj)
{
	M_BaseObject* tmp = hash(obj);
	std::size_t value = (std::size_t) unwrap_int(tmp);
	SAFE_DELETE(tmp);
	return value;
}

#define DEF_BINARY_OPER(name, lname, rname) \
	M_BaseObject* ObjSpace::##name(M_BaseObject* obj1, M_BaseObject* obj2) \
	{	\
		M_BaseObject* type1 = type(obj1);	\
		M_BaseObject* type2 = type(obj2);	\
		M_BaseObject* left_cls;		\
		M_BaseObject* left_impl = lookup_type_cls(type1, std::string(#lname), left_cls); 	\
		M_BaseObject* result = execute_binop(left_impl, obj1, obj2);	\
		return result;	\
	}

#define DEF_CMP_OPER(name, lname, rname) \
	M_BaseObject* ObjSpace::##name(M_BaseObject* obj1, M_BaseObject* obj2) \
	{	\
		M_BaseObject* type1 = type(obj1);	\
		M_BaseObject* type2 = type(obj2);	\
		M_BaseObject* left_cls;		\
		M_BaseObject* left_impl = lookup_type_cls(type1, std::string(#lname), left_cls); 	\
		M_BaseObject* result = execute_binop(left_impl, obj1, obj2);	\
		return result;	\
	}

DEF_BINARY_OPER(add, __add__, __radd__)

DEF_CMP_OPER(eq, __eq__, __eq__)

M_BaseObject* ObjSpace::new_interned_str(std::string& x)
{
	auto got = interned_str.find(x);
	if (got != interned_str.end()) return got->second;

	M_BaseObject* wrapped = wrap(x);
	interned_str[x] = wrapped;

	return wrapped;
}

M_BaseObject* ObjSpace::call_args(ThreadContext* context, M_BaseObject* func, Arguments& args)
{
	Function* as_func = dynamic_cast<Function*>(func);
	if (as_func) {
		return as_func->call_args(context, args);
	}

	return nullptr;
}

M_BaseObject* ObjSpace::get_and_call_function(ThreadContext* context, M_BaseObject* descr, std::initializer_list<M_BaseObject*> args)
{
	Function* as_func = dynamic_cast<Function*>(descr);
	if (as_func) {
		return call_function(context, descr, args);
	}

	return nullptr;
}

M_BaseObject* ObjSpace::call_function(ThreadContext* context, M_BaseObject* func, std::initializer_list<M_BaseObject*> args)
{
	Arguments arguments(context->get_space(), args);

	return call_args(context, func, arguments);
}

M_BaseObject* ObjSpace::getitem_str(M_BaseObject* obj, std::string& key)
{
	M_BaseObject* wrapped_key = wrap_str(key);
	M_BaseObject* value = getitem(obj, wrapped_key);
	SAFE_DELETE(wrapped_key);

	return value;
}

M_BaseObject* ObjSpace::getitem(M_BaseObject* obj, M_BaseObject* key)
{
	M_BaseObject* descr = lookup(obj, std::string("__getitem__"));

	if (!descr) throw InterpError(TypeError_type(), wrap_str("object is not subscriptable"));

	return get_and_call_function(current_thread(), descr, {obj, key});
}

void ObjSpace::setitem(M_BaseObject* obj, M_BaseObject* key, M_BaseObject* value)
{
	M_BaseObject* descr = lookup(obj, std::string("__setitem__"));

	if (!descr) throw InterpError(TypeError_type(), wrap_str("object item assignment not supported"));

	get_and_call_function(current_thread(), descr, {obj, key, value});
}
