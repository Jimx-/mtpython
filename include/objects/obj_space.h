#ifndef _OBJ_SPACE_H_
#define _OBJ_SPACE_H_

#include <string>
#include <initializer_list>
#include <unordered_map>
#include "objects/base_object.h"
#include "interpreter/arguments.h"
#include "objects/space_cache.h"
#include "exceptions.h"

#include <memory>

namespace mtpython {

namespace interpreter {
class BaseCompiler;
class Code;
class PyFrame;
} // namespace interpreter

namespace vm {
class ThreadContext;
class PyVM;
} // namespace vm

namespace gc {
class GarbageCollector;
}

namespace objects {

class ObjSpace {
private:
    /* Exception types */
    M_BaseObject* type_TypeError;
    M_BaseObject* type_StopIteration;
    M_BaseObject* type_NameError;
    M_BaseObject* type_UnboundLocalError;
    M_BaseObject* type_AttributeError;
    M_BaseObject* type_ImportError;
    M_BaseObject* type_ValueError;
    M_BaseObject* type_SystemError;
    M_BaseObject* type_KeyError;
    M_BaseObject* type_IndexError;
    M_BaseObject* type_SyntaxError;

    void init_builtin_exceptions();

protected:
    mtpython::vm::PyVM* vm;

    std::unique_ptr<TypedefCache> typedef_cache;
    GatewayCache gateway_cache;

    std::unordered_map<std::string, M_BaseObject*> interned_str;

    M_BaseObject* _io;
    M_BaseObject* builtin;
    M_BaseObject* sys;
    std::unordered_map<std::string, M_BaseObject*> builtin_modules;

    virtual void initialize() = 0;

    void make_builtins();
    void setup_builtin_modules();
    M_BaseObject* get_builtin_module(const std::string& name);
    M_BaseObject* execute_binop(M_BaseObject* impl, M_BaseObject* left,
                                M_BaseObject* right);

public:
    ObjSpace();
    virtual ~ObjSpace();

    void set_vm(mtpython::vm::PyVM* vm)
    {
        auto* old_vm = this->vm;
        this->vm = vm;
        if (!old_vm) initialize();
    }

    virtual void mark_roots(gc::GarbageCollector* gc);

    M_BaseObject* get__io() { return _io; }
    M_BaseObject* get_builtin() { return builtin; }
    M_BaseObject* get_sys() { return sys; }
    void set_builtin_module(std::string& name, M_BaseObject* mod)
    {
        builtin_modules[name] = mod;
    }

    M_BaseObject* get_gateway_cache(M_BaseObject* key)
    {
        return gateway_cache.get(key);
    }

    virtual interpreter::BaseCompiler* get_compiler(vm::ThreadContext* context);
    virtual interpreter::PyFrame* create_frame(vm::ThreadContext* context,
                                               interpreter::Code* code,
                                               M_BaseObject* globals,
                                               M_BaseObject* outer = nullptr)
    {
        throw NotImplementedException("Abstract");
    }

    virtual M_BaseObject* get_typeobject(interpreter::Typedef* def)
    {
        return typedef_cache->get(def);
    }
    virtual M_BaseObject* type(M_BaseObject* obj)
    {
        throw NotImplementedException("type()");
    }
    virtual std::string get_type_name(M_BaseObject* obj);

    virtual M_BaseObject* lookup(M_BaseObject* obj, const std::string& name)
    {
        throw NotImplementedException("lookup()");
    }
    virtual M_BaseObject* lookup_type_cls(M_BaseObject* obj,
                                          const std::string& attr,
                                          M_BaseObject*& cls)
    {
        throw NotImplementedException("lookup_type_cls()");
    }
    virtual M_BaseObject* lookup_type_starting_at(M_BaseObject* type,
                                                  M_BaseObject* start,
                                                  const std::string& name)
    {
        throw NotImplementedException("lookup_type_starting_at()");
    }
    virtual bool i_isinstance(M_BaseObject* obj, M_BaseObject* cls)
    {
        throw NotImplementedException("i_isinstance()");
    }
    virtual bool i_issubtype(M_BaseObject* sub, M_BaseObject* type)
    {
        throw NotImplementedException("i_issubtype()");
    }

    virtual M_BaseObject* get_type_by_name(const std::string& name)
    {
        throw NotImplementedException("get_type_by_name()");
    }

    M_BaseObject* TypeError_type() { return type_TypeError; }
    M_BaseObject* StopIteration_type() { return type_StopIteration; }
    M_BaseObject* NameError_type() { return type_NameError; }
    M_BaseObject* UnboundLocalError_type() { return type_UnboundLocalError; }
    M_BaseObject* AttributeError_type() { return type_AttributeError; }
    M_BaseObject* ImportError_type() { return type_ImportError; }
    M_BaseObject* ValueError_type() { return type_ValueError; }
    M_BaseObject* SystemError_type() { return type_SystemError; }
    M_BaseObject* KeyError_type() { return type_KeyError; }
    M_BaseObject* IndexError_type() { return type_IndexError; }
    M_BaseObject* SyntaxError_type() { return type_SyntaxError; }
    bool match_exception(M_BaseObject* type1, M_BaseObject* type2)
    {
        return type1 == type2;
    }

    virtual M_BaseObject* wrap(vm::ThreadContext* context, int x)
    {
        return wrap_int(context, x);
    }
    virtual M_BaseObject* wrap(vm::ThreadContext* context, const std::string& x)
    {
        return wrap_str(context, x);
    }
    virtual M_BaseObject* wrap(vm::ThreadContext* context, M_BaseObject* obj)
    {
        return obj;
    }

    virtual M_BaseObject* wrap_int(vm::ThreadContext* context, int x)
    {
        throw NotImplementedException("wrap_int()");
    }
    virtual M_BaseObject* wrap_int(vm::ThreadContext* context,
                                   const std::string& x)
    {
        throw NotImplementedException("wrap_int()");
    }

    virtual M_BaseObject* wrap_str(vm::ThreadContext* context,
                                   const std::string& x)
    {
        throw NotImplementedException("wrap_str()");
    }

    virtual M_BaseObject* wrap_None()
    {
        throw NotImplementedException("wrap_None()");
    }
    virtual M_BaseObject* wrap_True()
    {
        throw NotImplementedException("wrap_True()");
    }
    virtual M_BaseObject* wrap_False()
    {
        throw NotImplementedException("wrap_False()");
    }
    virtual M_BaseObject* wrap_NotImplemented()
    {
        throw NotImplementedException("wrap_NotImplemented()");
    }

    virtual M_BaseObject* new_bool(bool x)
    {
        if (x)
            return wrap_True();
        else
            return wrap_False();
    }
    virtual M_BaseObject* new_interned_str(const std::string& x);
    virtual M_BaseObject* new_tuple(vm::ThreadContext* context,
                                    const std::vector<M_BaseObject*>& items)
    {
        throw NotImplementedException("new_tuple()");
    }
    virtual M_BaseObject* new_list(vm::ThreadContext* context,
                                   const std::vector<M_BaseObject*>& items)
    {
        throw NotImplementedException("new_list()");
    }
    virtual M_BaseObject* new_dict(vm::ThreadContext* context)
    {
        throw NotImplementedException("new_dict()");
    }
    virtual M_BaseObject* new_set(vm::ThreadContext* context)
    {
        throw NotImplementedException("new_set()");
    }
    virtual M_BaseObject* new_seqiter(vm::ThreadContext* context,
                                      M_BaseObject* obj)
    {
        throw NotImplementedException("new_seqiter()");
    }

    virtual int unwrap_int(M_BaseObject* obj, bool allow_conversion = true);
    virtual std::string unwrap_str(M_BaseObject* obj)
    {
        return obj->to_string(this);
    }
    virtual void unwrap_tuple(M_BaseObject* obj,
                              std::vector<M_BaseObject*>& list)
    {}

    M_BaseObject* id(M_BaseObject* obj) { return obj->unique_id(this); }

    M_BaseObject* get(M_BaseObject* descr, M_BaseObject* obj,
                      M_BaseObject* type = nullptr);
    M_BaseObject* set(M_BaseObject* descr, M_BaseObject* obj,
                      M_BaseObject* value);

    M_BaseObject* get_and_call_args(vm::ThreadContext* context,
                                    M_BaseObject* descr, M_BaseObject* obj,
                                    interpreter::Arguments& args);
    M_BaseObject*
    get_and_call_function(vm::ThreadContext* context, M_BaseObject* descr,
                          const std::initializer_list<M_BaseObject*> args);
    M_BaseObject* call_args(vm::ThreadContext* context, M_BaseObject* func,
                            interpreter::Arguments& args);
    M_BaseObject* call_obj_args(vm::ThreadContext* context, M_BaseObject* func,
                                M_BaseObject* obj,
                                interpreter::Arguments& args);
    M_BaseObject*
    call_function(vm::ThreadContext* context, M_BaseObject* func,
                  const std::initializer_list<M_BaseObject*> args);

    M_BaseObject* getitem(M_BaseObject* obj, M_BaseObject* key);
    M_BaseObject* getitem_str(M_BaseObject* obj, const std::string& key);
    M_BaseObject* finditem(M_BaseObject* obj, M_BaseObject* key);
    M_BaseObject* finditem_str(M_BaseObject* obj, const std::string& key);
    void setitem(M_BaseObject* obj, M_BaseObject* key, M_BaseObject* value);
    void setitem_str(M_BaseObject* obj, const std::string& key,
                     M_BaseObject* value);
    M_BaseObject* delitem(M_BaseObject* obj, M_BaseObject* key);

    M_BaseObject* getattr(M_BaseObject* obj, M_BaseObject* name);
    M_BaseObject* getattr_str(M_BaseObject* obj, const std::string& name);
    M_BaseObject* findattr(M_BaseObject* obj, M_BaseObject* name);
    M_BaseObject* findattr_str(M_BaseObject* obj, const std::string& name);
    M_BaseObject* setattr(M_BaseObject* obj, M_BaseObject* name,
                          M_BaseObject* value);
    M_BaseObject* setattr_str(M_BaseObject* obj, const std::string& name,
                              M_BaseObject* value);
    M_BaseObject* delattr(M_BaseObject* obj, M_BaseObject* name);

    M_BaseObject* hash(M_BaseObject* obj);

    bool is_true(M_BaseObject* obj);
    bool i_is(M_BaseObject* obj1, M_BaseObject* obj2)
    {
        return (!obj2) ? false : obj2->i_is(this, obj1);
    }
    bool i_eq(M_BaseObject* obj1, M_BaseObject* obj2);
    std::size_t i_hash(M_BaseObject* obj);
    virtual int i_get_index(M_BaseObject* obj, M_BaseObject* exc,
                            M_BaseObject* descr);

    M_BaseObject* str(M_BaseObject* obj);
    M_BaseObject* repr(M_BaseObject* obj);

    M_BaseObject* iter(M_BaseObject* obj);
    M_BaseObject* next(M_BaseObject* obj);

    M_BaseObject* lt(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* le(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* gt(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* ge(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* ne(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* eq(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* contains(M_BaseObject* obj1, M_BaseObject* obj2);

    M_BaseObject* abs(M_BaseObject* obj);
    M_BaseObject* len(M_BaseObject* obj);

    M_BaseObject* pos(M_BaseObject* obj);
    M_BaseObject* neg(M_BaseObject* obj);
    M_BaseObject* not_(M_BaseObject* obj);
    M_BaseObject* invert(M_BaseObject* obj);

    M_BaseObject* add(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* sub(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* mul(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* truediv(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* floordiv(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* mod(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* lshift(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* rshift(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* and_(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* or_(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* xor_(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* pow(M_BaseObject* obj1, M_BaseObject* obj2);

    M_BaseObject* inplace_add(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* inplace_sub(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* inplace_mul(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* inplace_truediv(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* inplace_floordiv(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* inplace_mod(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* inplace_lshift(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* inplace_rshift(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* inplace_and(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* inplace_or(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* inplace_xor(M_BaseObject* obj1, M_BaseObject* obj2);
    M_BaseObject* inplace_pow(M_BaseObject* obj1, M_BaseObject* obj2);

    M_BaseObject* issubtype(M_BaseObject* sub, M_BaseObject* type);
    M_BaseObject* issubtype_override(M_BaseObject* sub, M_BaseObject* type);
    M_BaseObject* isinstance(M_BaseObject* obj, M_BaseObject* type);
    M_BaseObject* isinstance_override(M_BaseObject* obj, M_BaseObject* type);
};

} // namespace objects
} // namespace mtpython

#endif
