#include "vm/vm.h"
#include "interpreter/compiler.h"
#include "interpreter/function.h"
#include "interpreter/error.h"
#include "objects/space_cache.h"

#include "modules/_collections/collectionsmodule.h"
#include "modules/_io/iomodule.h"
#include "modules/builtins/bltinmodule.h"
#include "modules/posix/posixmodule.h"
#include "modules/sys/sysmodule.h"
#include "modules/_weakref/weakrefmodule.h"
#include "modules/errno/errnomodule.h"

#include "macros.h"

using namespace mtpython::objects;
using namespace mtpython::vm;
using namespace mtpython::interpreter;

ObjSpace::ObjSpace() : vm(nullptr), gateway_cache(this) {}

ObjSpace::~ObjSpace() {}

void ObjSpace::make_builtins()
{
    std::vector<M_BaseObject*> builtin_names;

    M_BaseObject* _io_name = wrap_str(ThreadContext::current_thread(), "_io");
    mtpython::modules::IOModule* io_mod =
        new mtpython::modules::IOModule(this, _io_name);
    io_mod->install();
    builtin_names.push_back(wrap_str(ThreadContext::current_thread(), "_io"));
    _io = io_mod;

    M_BaseObject* sys_name = wrap_str(ThreadContext::current_thread(), "sys");
    mtpython::modules::SysModule* sys_mod = new mtpython::modules::SysModule(
        ThreadContext::current_thread(), sys_name);
    sys_mod->install();
    builtin_names.push_back(wrap_str(ThreadContext::current_thread(), "sys"));
    sys = sys_mod;

    M_BaseObject* builtins_name =
        wrap_str(ThreadContext::current_thread(), "builtins");
    mtpython::modules::BuiltinsModule* builtins_mod =
        new mtpython::modules::BuiltinsModule(this, builtins_name);
    builtins_mod->install();
    builtin_names.push_back(
        wrap_str(ThreadContext::current_thread(), "builtins"));
    builtin = builtins_mod;
    setitem(builtins_mod->get_dict(this),
            wrap(ThreadContext::current_thread(), "__builtins__"),
            wrap(ThreadContext::current_thread(), builtins_mod));

    init_builtin_exceptions();

    M_BaseObject* posix_name =
        wrap_str(ThreadContext::current_thread(), "posix");
    mtpython::modules::PosixModule* posix_mod =
        new mtpython::modules::PosixModule(this, posix_name);
    posix_mod->install();
    builtin_names.push_back(wrap_str(ThreadContext::current_thread(), "posix"));

    M_BaseObject* _weakref_name =
        wrap_str(ThreadContext::current_thread(), "_weakref");
    mtpython::modules::WeakrefModule* weakref_mod =
        new mtpython::modules::WeakrefModule(this, _weakref_name);
    weakref_mod->install();
    builtin_names.push_back(
        wrap_str(ThreadContext::current_thread(), "_weakref"));

    M_BaseObject* errno_name =
        wrap_str(ThreadContext::current_thread(), "errno");
    mtpython::modules::ErrnoModule* errno_mod =
        new mtpython::modules::ErrnoModule(this, errno_name);
    errno_mod->install();
    builtin_names.push_back(wrap_str(ThreadContext::current_thread(), "errno"));

    M_BaseObject* _collections_name =
        wrap_str(ThreadContext::current_thread(), "_collections");
    mtpython::modules::CollectionsModule* collections_mod =
        new mtpython::modules::CollectionsModule(this, _collections_name);
    collections_mod->install();
    builtin_names.push_back(
        wrap_str(ThreadContext::current_thread(), "_collections"));

    setitem(sys_mod->get_dict(this),
            wrap_str(ThreadContext::current_thread(), "builtin_module_names"),
            new_tuple(ThreadContext::current_thread(), builtin_names));
}

void ObjSpace::setup_builtin_modules()
{
    get_builtin_module("_io");
    get_builtin_module("sys");
    get_builtin_module("builtins");
    get_builtin_module("posix");
    get_builtin_module("_weakref");
    get_builtin_module("errno");
    get_builtin_module("_collections");
}

void ObjSpace::init_builtin_exceptions()
{
#define SET_EXCEPTION_TYPE(name) \
    type_##name = builtin->get_dict_value(this, #name);
    SET_EXCEPTION_TYPE(TypeError);
    SET_EXCEPTION_TYPE(StopIteration);
    SET_EXCEPTION_TYPE(NameError);
    SET_EXCEPTION_TYPE(UnboundLocalError);
    SET_EXCEPTION_TYPE(AttributeError);
    SET_EXCEPTION_TYPE(ImportError);
    SET_EXCEPTION_TYPE(ValueError);
    SET_EXCEPTION_TYPE(SystemError);
    SET_EXCEPTION_TYPE(KeyError);
    SET_EXCEPTION_TYPE(IndexError);
    SET_EXCEPTION_TYPE(SyntaxError);
}

void ObjSpace::mark_roots(gc::GarbageCollector* gc)
{
    for (const auto& [k, v] : interned_str)
        gc->mark_object(v);

    typedef_cache->mark_objects(gc);
    gateway_cache.mark_objects(gc);
}

M_BaseObject* ObjSpace::get_builtin_module(const std::string& name)
{
    M_BaseObject* wrapped_name =
        wrap_str(ThreadContext::current_thread(), name);
    M_BaseObject* sys_modules = sys->get(this, "modules");

    auto got = builtin_modules.find(name);
    if (got == builtin_modules.end()) {
        throw InterpError::format(this, SystemError_type(),
                                  "try to get non-builtin module %s",
                                  name.c_str());
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

M_BaseObject* ObjSpace::execute_binop(M_BaseObject* impl, M_BaseObject* left,
                                      M_BaseObject* right)
{
    Function* func = dynamic_cast<Function*>(impl);
    if (!func) return nullptr;

    return call_function(ThreadContext::current_thread(), func, {left, right});
}

M_BaseObject* ObjSpace::hash(M_BaseObject* obj)
{
    M_BaseObject* descr = lookup(obj, "__hash__");
    if (!descr) {
        return obj->unique_id(this);
    }

    M_BaseObject* hash_value =
        get_and_call_function(ThreadContext::current_thread(), descr, {obj});

    return hash_value;
}

bool ObjSpace::is_true(M_BaseObject* obj)
{
    M_BaseObject* descr = lookup(obj, "__bool__");
    if (!descr) {
        descr = lookup(obj, "__len__");
        if (!descr) return true;

        M_BaseObject* result = get_and_call_function(
            ThreadContext::current_thread(), descr, {obj});
        return unwrap_int(result) != 0;
    }

    M_BaseObject* result =
        get_and_call_function(ThreadContext::current_thread(), descr, {obj});

    if (i_is(result, wrap_True())) return true;
    if (i_is(result, wrap_False())) return false;

    throw InterpError::format(this, TypeError_type(),
                              "__bool__ should return bool, return %s",
                              get_type_name(result).c_str());

    return false;
}

bool ObjSpace::i_eq(M_BaseObject* obj1, M_BaseObject* obj2)
{
    return i_is(obj1, obj2) || is_true(eq(obj1, obj2));
}

std::size_t ObjSpace::i_hash(M_BaseObject* obj)
{
    M_BaseObject* tmp = hash(obj);
    std::size_t value = (std::size_t)unwrap_int(tmp);
    return value;
}

#define DEF_BINARY_OPER(name, lname, rname)                                 \
    M_BaseObject* ObjSpace::name(M_BaseObject* obj1, M_BaseObject* obj2)    \
    {                                                                       \
        M_BaseObject* type1 = type(obj1);                                   \
        M_BaseObject* type2 = type(obj2);                                   \
        M_BaseObject* left_cls;                                             \
        M_BaseObject* left_impl = lookup_type_cls(type1, #lname, left_cls); \
        if (!left_impl)                                                     \
            throw InterpError::format(                                      \
                this, TypeError_type(),                                     \
                "unsupported operand type(s) for %s: '%s' and '%s'", #name, \
                get_type_name(obj1).c_str(), get_type_name(obj2).c_str());  \
        M_BaseObject* result = execute_binop(left_impl, obj1, obj2);        \
        return result;                                                      \
    }

#define DEF_UNARY_OPER(name, special_name)                                  \
    M_BaseObject* ObjSpace::name(M_BaseObject* obj)                         \
    {                                                                       \
        M_BaseObject* impl = lookup(obj, #special_name);                    \
        if (!impl)                                                          \
            throw InterpError::format(                                      \
                this, TypeError_type(),                                     \
                "unsupported operand type for unary %s: '%s'", #name,       \
                get_type_name(obj).c_str());                                \
        return get_and_call_function(ThreadContext::current_thread(), impl, \
                                     {obj});                                \
    }

#define DEF_CMP_OPER(name, lname, rname, symbol)                            \
    M_BaseObject* ObjSpace::name(M_BaseObject* obj1, M_BaseObject* obj2)    \
    {                                                                       \
        M_BaseObject* type1 = type(obj1);                                   \
        M_BaseObject* type2 = type(obj2);                                   \
        M_BaseObject* left_cls;                                             \
        M_BaseObject* left_impl = lookup_type_cls(type1, #lname, left_cls); \
        if (!left_impl)                                                     \
            throw InterpError::format(this, TypeError_type(),               \
                                      "unorderable types: '%s' %s '%s'",    \
                                      get_type_name(obj1).c_str(), #symbol, \
                                      get_type_name(obj2).c_str());         \
        M_BaseObject* result = execute_binop(left_impl, obj1, obj2);        \
        return result;                                                      \
    }

DEF_BINARY_OPER(add, __add__, __radd__)
DEF_BINARY_OPER(sub, __sub__, __rsub__)
DEF_BINARY_OPER(mul, __mul__, __rmul__)
DEF_BINARY_OPER(truediv, __truediv__, __rtruediv__)
DEF_BINARY_OPER(floordiv, __floodiv__, __rfloordiv__)
DEF_BINARY_OPER(mod, __mod__, __rmod__)
DEF_BINARY_OPER(lshift, __lshift__, __rlshift__)
DEF_BINARY_OPER(rshift, __rshift__, __rrshift__)
DEF_BINARY_OPER(and_, __and__, __rand__)
DEF_BINARY_OPER(or_, __or__, __ror__)
DEF_BINARY_OPER(xor_, __xor__, __rxor__)
DEF_BINARY_OPER(pow, __pow__, __rpow__)

DEF_BINARY_OPER(inplace_add, __iadd__, __iadd__)
DEF_BINARY_OPER(inplace_sub, __isub__, __isub__)
DEF_BINARY_OPER(inplace_mul, __imul__, __imul__)
DEF_BINARY_OPER(inplace_truediv, __itruediv__, __itruediv__)
DEF_BINARY_OPER(inplace_floordiv, __ifloodiv__, __ifloordiv__)
DEF_BINARY_OPER(inplace_mod, __imod__, __imod__)
DEF_BINARY_OPER(inplace_lshift, __ilshift__, __ilshift__)
DEF_BINARY_OPER(inplace_rshift, __irshift__, __irshift__)
DEF_BINARY_OPER(inplace_and, __iand__, __iand__)
DEF_BINARY_OPER(inplace_or, __ior__, __ior__)
DEF_BINARY_OPER(inplace_xor, __ixor__, __ixor__)
DEF_BINARY_OPER(inplace_pow, __ipow__, __ipow__)

DEF_UNARY_OPER(pos, __pos__)
DEF_UNARY_OPER(neg, __neg__)
DEF_UNARY_OPER(invert, __invert__)

M_BaseObject* ObjSpace::eq(M_BaseObject* obj1, M_BaseObject* obj2)
{
    M_BaseObject* type1 = type(obj1);
    M_BaseObject* type2 = type(obj2);
    M_BaseObject* left_cls;
    M_BaseObject* left_impl = lookup_type_cls(type1, "__eq__", left_cls);
    if (!left_impl) return new_bool(i_is(obj1, obj2));
    M_BaseObject* result = execute_binop(left_impl, obj1, obj2);
    return result;
}

M_BaseObject* ObjSpace::ne(M_BaseObject* obj1, M_BaseObject* obj2)
{
    M_BaseObject* type1 = type(obj1);
    M_BaseObject* type2 = type(obj2);
    M_BaseObject* left_cls;
    M_BaseObject* left_impl = lookup_type_cls(type1, "__ne__", left_cls);
    if (!left_impl) return not_(eq(obj1, obj2));
    M_BaseObject* result = execute_binop(left_impl, obj1, obj2);
    return result;
}

DEF_CMP_OPER(lt, __lt__, __lt__, <)
DEF_CMP_OPER(le, __le__, __le__, <=)
DEF_CMP_OPER(gt, __gt__, __gt__, >)
DEF_CMP_OPER(ge, __ge__, __ge__, >=)
DEF_CMP_OPER(contains, __contains__, __contains__, in)

int ObjSpace::unwrap_int(M_BaseObject* obj, bool allow_conversion)
{
    int result;
    try {
        result = obj->to_int(this, allow_conversion);
    } catch (const NotImplementedException&) {
        throw InterpError::format(
            this, TypeError_type(),
            "'%s' object cannot be converted to interpreter-level integer",
            get_type_name(obj).c_str());
    }

    return result;
}

M_BaseObject* ObjSpace::not_(M_BaseObject* obj)
{
    return new_bool(!is_true(obj));
}

M_BaseObject* ObjSpace::str(M_BaseObject* obj)
{
    M_BaseObject* descr = lookup(obj, "__str__");
    if (!descr) {
        throw InterpError::format(this, TypeError_type(),
                                  "unsupported operand type for str '%s'",
                                  get_type_name(obj).c_str());
    }
    return get_and_call_function(ThreadContext::current_thread(), descr, {obj});
}

M_BaseObject* ObjSpace::repr(M_BaseObject* obj)
{
    M_BaseObject* descr = lookup(obj, "__repr__");
    if (!descr) {
        throw InterpError::format(this, TypeError_type(),
                                  "unsupported operand type for repr '%s'",
                                  get_type_name(obj).c_str());
    }
    return get_and_call_function(ThreadContext::current_thread(), descr, {obj});
}

M_BaseObject* ObjSpace::iter(M_BaseObject* obj)
{
    M_BaseObject* descr = lookup(obj, "__iter__");
    if (!descr) {
        descr = lookup(obj, "__getitem__");
        if (!descr)
            throw InterpError::format(this, TypeError_type(),
                                      "'%s' object is not iterable",
                                      get_type_name(obj).c_str());
    }

    M_BaseObject* iterator =
        get_and_call_function(ThreadContext::current_thread(), descr, {obj});
    M_BaseObject* next = lookup(iterator, "__next__");
    if (!next)
        throw InterpError(TypeError_type(),
                          wrap_str(ThreadContext::current_thread(),
                                   "iter() returned non-iterator"));

    return iterator;
}

M_BaseObject* ObjSpace::next(M_BaseObject* obj)
{
    M_BaseObject* descr = lookup(obj, "__next__");
    if (!descr) {
        throw InterpError::format(this, TypeError_type(),
                                  "'%s' object is not an iterator",
                                  get_type_name(obj).c_str());
    }
    return get_and_call_function(ThreadContext::current_thread(), descr, {obj});
}

M_BaseObject* ObjSpace::new_interned_str(const std::string& x)
{
    auto got = interned_str.find(x);
    if (got != interned_str.end()) return got->second;

    M_BaseObject* wrapped = wrap(ThreadContext::current_thread(), x);
    interned_str[x] = wrapped;

    return wrapped;
}

M_BaseObject* ObjSpace::get_and_call_args(ThreadContext* context,
                                          M_BaseObject* descr,
                                          M_BaseObject* obj, Arguments& args)
{
    Function* as_func = dynamic_cast<Function*>(descr);
    if (as_func) {
        return as_func->call_obj_args(context, obj, args);
    }

    return nullptr;
}

M_BaseObject* ObjSpace::call_args(ThreadContext* context, M_BaseObject* func,
                                  Arguments& args)
{
    Function* as_func = dynamic_cast<Function*>(func);
    if (as_func) {
        return as_func->call_args(context, args);
    }

    Method* as_method = dynamic_cast<Method*>(func);
    if (as_method) {
        return as_method->call_args(context, args);
    }
    M_BaseObject* descr = lookup(func, "__call__");
    if (!descr) {
        throw InterpError::format(this, TypeError_type(),
                                  "'%s' object is not callable",
                                  get_type_name(func).c_str());
    }

    return get_and_call_args(context, descr, func, args);
}

M_BaseObject* ObjSpace::call_obj_args(ThreadContext* context,
                                      M_BaseObject* func, M_BaseObject* obj,
                                      Arguments& args)
{
    Function* as_func = dynamic_cast<Function*>(func);
    if (as_func) {
        return as_func->call_obj_args(context, obj, args);
    }

    return nullptr;
}

M_BaseObject* ObjSpace::get(M_BaseObject* descr, M_BaseObject* obj,
                            M_BaseObject* type)
{
    M_BaseObject* getter = lookup(descr, "__get__");
    if (!getter) return descr;

    if (!type) type = this->type(obj);
    return get_and_call_function(ThreadContext::current_thread(), getter,
                                 {descr, obj, type});
}

M_BaseObject* ObjSpace::set(M_BaseObject* descr, M_BaseObject* obj,
                            M_BaseObject* value)
{
    M_BaseObject* setter = lookup(descr, "__set__");
    if (!setter) {
        throw InterpError::format(this, TypeError_type(),
                                  "'%s' object is not a descriptor with set",
                                  get_type_name(descr).c_str());
    }
    return get_and_call_function(ThreadContext::current_thread(), setter,
                                 {descr, obj, value});
}

M_BaseObject*
ObjSpace::get_and_call_function(ThreadContext* context, M_BaseObject* descr,
                                const std::initializer_list<M_BaseObject*> args)
{
    Function* as_func = dynamic_cast<Function*>(descr);
    if (as_func) {
        return call_function(context, descr, args);
    }

    return nullptr;
}

M_BaseObject*
ObjSpace::call_function(ThreadContext* context, M_BaseObject* func,
                        const std::initializer_list<M_BaseObject*> args)
{
    Arguments arguments(context->get_space(), args);

    return call_args(context, func, arguments);
}

M_BaseObject* ObjSpace::getitem_str(M_BaseObject* obj, const std::string& key)
{
    M_BaseObject* wrapped_key = wrap_str(ThreadContext::current_thread(), key);
    M_BaseObject* value = getitem(obj, wrapped_key);

    return value;
}

M_BaseObject* ObjSpace::getitem(M_BaseObject* obj, M_BaseObject* key)
{
    M_BaseObject* descr = lookup(obj, "__getitem__");

    if (!descr)
        throw InterpError::format(this, TypeError_type(),
                                  "'%s' object is not subscriptable",
                                  get_type_name(obj).c_str());

    return get_and_call_function(ThreadContext::current_thread(), descr,
                                 {obj, key});
}

M_BaseObject* ObjSpace::finditem_str(M_BaseObject* obj, const std::string& key)
{
    M_BaseObject* wrapped_key = wrap_str(ThreadContext::current_thread(), key);
    M_BaseObject* value = finditem(obj, wrapped_key);

    return value;
}

M_BaseObject* ObjSpace::finditem(M_BaseObject* obj, M_BaseObject* key)
{
    M_BaseObject* value;
    try {
        value = getitem(obj, key);
    } catch (InterpError& exc) {
        if (exc.match(this, type_KeyError)) return nullptr;
        throw exc;
    }

    return value;
}

void ObjSpace::setitem_str(M_BaseObject* obj, const std::string& key,
                           M_BaseObject* value)
{
    M_BaseObject* wrapped_key = wrap_str(ThreadContext::current_thread(), key);
    setitem(obj, wrapped_key, value);
}

void ObjSpace::setitem(M_BaseObject* obj, M_BaseObject* key,
                       M_BaseObject* value)
{
    M_BaseObject* descr = lookup(obj, "__setitem__");

    if (!descr)
        throw InterpError::format(
            this, TypeError_type(),
            "'%s' object does not support item assignment",
            get_type_name(obj).c_str());

    get_and_call_function(ThreadContext::current_thread(), descr,
                          {obj, key, value});
}

M_BaseObject* ObjSpace::delitem(M_BaseObject* obj, M_BaseObject* key)
{
    M_BaseObject* descr = lookup(obj, "__delitem__");

    if (!descr)
        throw InterpError::format(this, TypeError_type(),
                                  "'%s' object does not support item deletion",
                                  get_type_name(obj).c_str());

    return get_and_call_function(ThreadContext::current_thread(), descr,
                                 {obj, key});
}

M_BaseObject* ObjSpace::getattr(M_BaseObject* obj, M_BaseObject* name)
{
    M_BaseObject* descr = lookup(obj, "__getattribute__");
    try {
        if (!descr) throw InterpError(AttributeError_type(), wrap_None());
        return get_and_call_function(ThreadContext::current_thread(), descr,
                                     {obj, name});
    } catch (InterpError& e) {
        if (!e.match(this, AttributeError_type())) throw e;
        /* TODO: delete e.value */
        descr = lookup(obj, "__getattr__");
        if (!descr) throw e;
    }
    return get_and_call_function(ThreadContext::current_thread(), descr,
                                 {obj, name});
}

M_BaseObject* ObjSpace::getattr_str(M_BaseObject* obj, const std::string& name)
{
    M_BaseObject* wrapped_name =
        wrap_str(ThreadContext::current_thread(), name);
    M_BaseObject* value = getattr(obj, wrapped_name);

    return value;
}

M_BaseObject* ObjSpace::findattr(M_BaseObject* obj, M_BaseObject* name)
{
    M_BaseObject* value;
    try {
        value = getattr(obj, name);
    } catch (InterpError& exc) {
        if (exc.match(this, type_AttributeError)) return nullptr;
        throw exc;
    }

    return value;
}

M_BaseObject* ObjSpace::findattr_str(M_BaseObject* obj, const std::string& name)
{
    M_BaseObject* wrapped_name =
        wrap_str(ThreadContext::current_thread(), name);
    M_BaseObject* value = findattr(obj, wrapped_name);

    return value;
}

M_BaseObject* ObjSpace::setattr(M_BaseObject* obj, M_BaseObject* name,
                                M_BaseObject* value)
{
    M_BaseObject* descr = lookup(obj, "__setattr__");
    if (!descr)
        throw InterpError::format(this, TypeError_type(),
                                  "'%s' object is readonly",
                                  get_type_name(obj).c_str());
    return get_and_call_function(ThreadContext::current_thread(), descr,
                                 {obj, name, value});
}

M_BaseObject* ObjSpace::setattr_str(M_BaseObject* obj, const std::string& name,
                                    M_BaseObject* value)
{
    M_BaseObject* wrapped_name =
        wrap_str(ThreadContext::current_thread(), name);
    M_BaseObject* result = setattr(obj, wrapped_name, value);

    return result;
}

M_BaseObject* ObjSpace::delattr(M_BaseObject* obj, M_BaseObject* name)
{
    M_BaseObject* descr = lookup(obj, "__delattr__");
    if (!descr)
        throw InterpError::format(
            this, TypeError_type(),
            "'%s' object does not support attribute removal",
            get_type_name(obj).c_str());
    return get_and_call_function(ThreadContext::current_thread(), descr,
                                 {obj, name});
}

M_BaseObject* ObjSpace::abs(M_BaseObject* obj)
{
    M_BaseObject* descr = lookup(obj, "__abs__");

    if (!descr) return nullptr;

    return get_and_call_function(ThreadContext::current_thread(), descr, {obj});
}

M_BaseObject* ObjSpace::len(M_BaseObject* obj)
{
    M_BaseObject* descr = lookup(obj, "__len__");

    if (!descr) return nullptr;

    return get_and_call_function(ThreadContext::current_thread(), descr, {obj});
}

M_BaseObject* ObjSpace::issubtype(M_BaseObject* sub, M_BaseObject* type)
{
    return new_bool(i_issubtype(sub, type));
}

M_BaseObject* ObjSpace::issubtype_override(M_BaseObject* sub,
                                           M_BaseObject* type)
{
    M_BaseObject* descr = lookup(type, "__subclasscheck__");
    if (descr) {
        return get_and_call_function(ThreadContext::current_thread(), descr,
                                     {type, sub});
    }

    throw InterpError(
        TypeError_type(),
        wrap_str(ThreadContext::current_thread(), "issubclass not supported"));
}

M_BaseObject* ObjSpace::isinstance(M_BaseObject* obj, M_BaseObject* type)
{
    return new_bool(i_isinstance(obj, type));
}

M_BaseObject* ObjSpace::isinstance_override(M_BaseObject* obj,
                                            M_BaseObject* type)
{
    M_BaseObject* descr = lookup(type, "__instancecheck__");
    if (descr) {
        return get_and_call_function(ThreadContext::current_thread(), descr,
                                     {type, obj});
    }

    return isinstance(obj, type);
}

int ObjSpace::i_get_index(M_BaseObject* obj, M_BaseObject* exc,
                          M_BaseObject* descr)
{
    throw InterpError(exc, descr);
    return 0;
}
