#include "vm/vm.h"
#include "interpreter/compiler.h"
#include "interpreter/function.h"
#include "interpreter/error.h"

#include "modules/builtins/bltinmodule.h"
#include "modules/sys/sysmodule.h"

#include "macros.h"

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
	M_BaseObject* sys_name = wrap_str("sys");
	mtpython::modules::SysModule* sys_mod = new mtpython::modules::SysModule(this, sys_name);
	sys_mod->install();
	sys = sys_mod;

	M_BaseObject* builtins_name = wrap_str("builtins");
	mtpython::modules::BuiltinsModule* builtins_mod = new mtpython::modules::BuiltinsModule(this, builtins_name);
	builtins_mod->install();
	builtin = builtins_mod;
	setitem(builtins_mod->get_dict(), wrap("__builtins__"), wrap(builtins_mod));

	init_builtin_exceptions();
}

void ObjSpace::setup_builtin_modules()
{
	get_builtin_module("sys");
	get_builtin_module("builtins");
}

void ObjSpace::init_builtin_exceptions()
{
#define SET_EXCEPTION_TYPE(name) type_##name = builtin->get_dict_value(this, #name);
	SET_EXCEPTION_TYPE(TypeError);
    SET_EXCEPTION_TYPE(StopIteration);
	SET_EXCEPTION_TYPE(NameError);
	SET_EXCEPTION_TYPE(UnboundLocalError);
	SET_EXCEPTION_TYPE(AttributeError);
	SET_EXCEPTION_TYPE(ImportError);
	SET_EXCEPTION_TYPE(ValueError);
	SET_EXCEPTION_TYPE(SystemError);
}

M_BaseObject* ObjSpace::get_builtin_module(const std::string& name)
{
	M_BaseObject* wrapped_name = wrap_str(name);
	M_BaseObject* sys_modules = sys->get(this, "modules");

	auto got = builtin_modules.find(name);
	if (got == builtin_modules.end()) {
		throw InterpError::format(this, SystemError_type(), "try to get non-builtin module %s", name.c_str());
	}

	setitem(sys_modules, wrapped_name, got->second);

	return got->second;
}

BaseCompiler* ObjSpace::get_compiler(ThreadContext* context)
{
	return new PyCompiler(context);
}

std::string ObjSpace::get_type_name(M_BaseObject* obj)
{
	Typedef* def = obj->get_typedef();
	if (!def) return "";

	return def->get_name();
}

M_BaseObject* ObjSpace::execute_binop(M_BaseObject* impl, M_BaseObject* left, M_BaseObject* right)
{
	Function* func = dynamic_cast<Function*>(impl);
	if (!func) return nullptr;

	return call_function(current_thread(), func, {left, right});
}	

M_BaseObject* ObjSpace::hash(M_BaseObject* obj)
{
	M_BaseObject* descr = lookup(obj, "__hash__");
	if (!descr) {
		//return wrap((unsigned int)obj);
	}

	M_BaseObject* hash_value = get_and_call_function(current_thread(), descr, {obj});

	return hash_value;
}

bool ObjSpace::is_true(M_BaseObject* obj)
{
	M_BaseObject* descr = lookup(obj, "__bool__");
	if (!descr) return true;

	M_BaseObject* result = get_and_call_function(current_thread(), descr, {obj});

	if (i_is(result, wrap_True())) return true;
	if (i_is(result, wrap_False())) return false;

	throw InterpError::format(this, TypeError_type(), "__bool__ should return bool, return %s", get_type_name(result).c_str());

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
	M_BaseObject* ObjSpace::name(M_BaseObject* obj1, M_BaseObject* obj2) \
	{	\
		M_BaseObject* type1 = type(obj1);	\
		M_BaseObject* type2 = type(obj2);	\
		M_BaseObject* left_cls;		\
		M_BaseObject* left_impl = lookup_type_cls(type1, #lname, left_cls); 	\
		M_BaseObject* result = execute_binop(left_impl, obj1, obj2);	\
		return result;	\
	}

#define DEF_UNARY_OPER(name, special_name) \
	M_BaseObject* ObjSpace::name(M_BaseObject* obj) \
	{	\
		M_BaseObject* impl = lookup(obj, #special_name);	\
 		if (!impl) throw InterpError::format(this, TypeError_type(), "unsupported operand type for unary %s: '%s'", #name, get_type_name(obj).c_str());	\
		return get_and_call_function(current_thread(), impl, {obj});	\
	}

#define DEF_CMP_OPER(name, lname, rname) \
	M_BaseObject* ObjSpace::name(M_BaseObject* obj1, M_BaseObject* obj2) \
	{	\
		M_BaseObject* type1 = type(obj1);	\
		M_BaseObject* type2 = type(obj2);	\
		M_BaseObject* left_cls;		\
		M_BaseObject* left_impl = lookup_type_cls(type1, #lname, left_cls); 	\
		M_BaseObject* result = execute_binop(left_impl, obj1, obj2);	\
		return result;	\
	}

DEF_BINARY_OPER(add, __add__, __radd__)
DEF_BINARY_OPER(sub, __sub__, __rsub__)
DEF_BINARY_OPER(mul, __mul__, __rmul__)

DEF_UNARY_OPER(pos, __pos__)
DEF_UNARY_OPER(neg, __neg__)
DEF_UNARY_OPER(invert, __invert__)

DEF_CMP_OPER(eq, __eq__, __eq__)
DEF_CMP_OPER(lt, __lt__, __lt__)
DEF_CMP_OPER(le, __le__, __le__)
DEF_CMP_OPER(gt, __gt__, __gt__)
DEF_CMP_OPER(ge, __ge__, __ge__)
DEF_CMP_OPER(ne, __ne__, __ne__)

M_BaseObject* ObjSpace::not_(M_BaseObject* obj)
{
	return new_bool(!is_true(obj));
}

M_BaseObject* ObjSpace::str(M_BaseObject* obj)
{
	M_BaseObject* descr = lookup(obj, "__str__");
	if (!descr) {
		throw InterpError::format(this, TypeError_type(), "unsupported operand type for str '%s'", get_type_name(obj).c_str());
	}
	return get_and_call_function(current_thread(), descr, {obj});
}

M_BaseObject* ObjSpace::repr(M_BaseObject* obj)
{
	M_BaseObject* descr = lookup(obj, "__repr__");
	if (!descr) {
		throw InterpError::format(this, TypeError_type(), "unsupported operand type for repr '%s'", get_type_name(obj).c_str());
	}
	return get_and_call_function(current_thread(), descr, {obj});
}

M_BaseObject* ObjSpace::iter(M_BaseObject* obj)
{
	M_BaseObject* descr = lookup(obj, "__iter__");
    if (!descr) {
        descr = lookup(obj, "__getitem__");
        if (!descr) throw InterpError::format(this, TypeError_type(), "'%s' object is not iterable", get_type_name(obj).c_str());
    }

    M_BaseObject* iterator = get_and_call_function(current_thread(), descr, {obj});
    M_BaseObject* next = lookup(iterator, "__next__");
    if (!next) throw InterpError(TypeError_type(), wrap_str("iter() returned non-iterator"));

    return iterator;
}

M_BaseObject* ObjSpace::next(M_BaseObject* obj)
{
    M_BaseObject* descr = lookup(obj, "__next__");
    if (!descr) {
		throw InterpError::format(this, TypeError_type(), "'%s' object is not an iterator", get_type_name(obj).c_str());
    }
    return get_and_call_function(current_thread(), descr, {obj});
}

M_BaseObject* ObjSpace::new_interned_str(const std::string& x)
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

	Method* as_method = dynamic_cast<Method*>(func);
	if (as_method) {
		return as_method->call_args(context, args);
	}

	return nullptr;
}

M_BaseObject* ObjSpace::call_obj_args(ThreadContext* context, M_BaseObject* func, M_BaseObject* obj, Arguments& args)
{
	Function* as_func = dynamic_cast<Function*>(func);
	if (as_func) {
		return as_func->call_obj_args(context, obj, args);
	}

	return nullptr;
}

M_BaseObject* ObjSpace::get(M_BaseObject* descr, M_BaseObject* obj, M_BaseObject* type)
{
	M_BaseObject* getter = lookup(descr, "__get__");
	if (!getter) return descr;

	if (!type) type = this->type(obj);
	return get_and_call_function(current_thread(), getter, {descr, obj, type});
}

M_BaseObject* ObjSpace::get_and_call_function(ThreadContext* context, M_BaseObject* descr, const std::initializer_list<M_BaseObject*> args)
{
	Function* as_func = dynamic_cast<Function*>(descr);
	if (as_func) {
		return call_function(context, descr, args);
	}

	return nullptr;
}

M_BaseObject* ObjSpace::call_function(ThreadContext* context, M_BaseObject* func, const std::initializer_list<M_BaseObject*> args)
{
	Arguments arguments(context->get_space(), args);

	return call_args(context, func, arguments);
}

M_BaseObject* ObjSpace::getitem_str(M_BaseObject* obj, const std::string& key)
{
	M_BaseObject* wrapped_key = wrap_str(key);
	M_BaseObject* value = getitem(obj, wrapped_key);
	SAFE_DELETE(wrapped_key);

	return value;
}

M_BaseObject* ObjSpace::getitem(M_BaseObject* obj, M_BaseObject* key)
{
	M_BaseObject* descr = lookup(obj, "__getitem__");

	if (!descr) throw InterpError::format(this, TypeError_type(), "'%s' object is not subscriptable", get_type_name(obj).c_str());

	return get_and_call_function(current_thread(), descr, {obj, key});
}

void ObjSpace::setitem_str(M_BaseObject* obj, const std::string& key, M_BaseObject* value)
{
	M_BaseObject* wrapped_key = wrap_str(key);
	setitem(obj, wrapped_key, value);
}

void ObjSpace::setitem(M_BaseObject* obj, M_BaseObject* key, M_BaseObject* value)
{
	M_BaseObject* descr = lookup(obj, "__setitem__");

	if (!descr) throw InterpError::format(this, TypeError_type(), "'%s' object does not support item assignment", get_type_name(obj).c_str());

	get_and_call_function(current_thread(), descr, {obj, key, value});
}

M_BaseObject* ObjSpace::delitem(M_BaseObject* obj, M_BaseObject* key)
{
	M_BaseObject* descr = lookup(obj, "__delitem__");

	if (!descr) throw InterpError::format(this, TypeError_type(), "'%s' object does not support item deletion", get_type_name(obj).c_str());

	return get_and_call_function(current_thread(), descr, {obj, key});
}

M_BaseObject* ObjSpace::getattr(M_BaseObject* obj, M_BaseObject* name)
{
	M_BaseObject* descr = lookup(obj, "__getattribute__");
	try {
		if (!descr) throw InterpError(AttributeError_type(), wrap_None());
		return get_and_call_function(current_thread(), descr, {obj, name});
	} catch (InterpError& e) {
		if (!e.match(this, AttributeError_type())) throw e;
		/* TODO: delete e.value */
		descr = lookup(obj, "__getattr__");
		if (!descr) throw e;
	}
	return get_and_call_function(current_thread(), descr, {obj, name});
}

M_BaseObject* ObjSpace::setattr(M_BaseObject* obj, M_BaseObject* name, M_BaseObject* value)
{
	M_BaseObject* descr = lookup(obj, "__setattr__");
	if (!descr) throw InterpError::format(this, TypeError_type(), "'%s' object is readonly", get_type_name(obj).c_str());
	return get_and_call_function(current_thread(), descr, {obj, name});
}

M_BaseObject* ObjSpace::delattr(M_BaseObject* obj, M_BaseObject* name)
{
	M_BaseObject* descr = lookup(obj, "__delattr__");
	if (!descr) throw InterpError::format(this, TypeError_type(), "'%s' object does not support attribute removal", get_type_name(obj).c_str());
	return get_and_call_function(current_thread(), descr, {obj, name});
}

M_BaseObject* ObjSpace::abs(M_BaseObject* obj)
{
	M_BaseObject* descr = lookup(obj, "__abs__");

	if (!descr) return nullptr;

	return get_and_call_function(current_thread(), descr, {obj});
}

M_BaseObject* ObjSpace::len(M_BaseObject* obj)
{
	M_BaseObject* descr = lookup(obj, "__len__");

	if (!descr) return nullptr;

	return get_and_call_function(current_thread(), descr, {obj});
}

