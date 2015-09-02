#ifndef _OBJ_SPACE_H_
#define _OBJ_SPACE_H_

#include <string>
#include <initializer_list>
#include <unordered_map>
#include "objects/base_object.h"
#include "interpreter/arguments.h"
#include "objects/space_cache.h"
#include "exceptions.h"

namespace mtpython {

namespace interpreter {
class BaseCompiler;
class Code;
class PyFrame;
}

namespace vm {
class ThreadContext;
class PyVM;
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

	void init_builtin_exceptions();
protected:
	mtpython::vm::PyVM* vm;
	vm::ThreadContext* dummy_context;	/* used only in initialization, when VM is not set */

	TypedefCache* typedef_cache;
	GatewayCache gateway_cache;

	std::unordered_map<std::string, M_BaseObject*> interned_str;

	M_BaseObject* builtin;
	std::unordered_map<std::string, M_BaseObject*> builtin_modules;

	vm::ThreadContext* current_thread();

	M_BaseObject* execute_binop(M_BaseObject* impl, M_BaseObject* left, M_BaseObject* right);
public:
	ObjSpace();
	~ObjSpace();

	void set_vm(mtpython::vm::PyVM* vm) { this->vm = vm; }

	M_BaseObject* get_builtin() { return builtin; }
	void make_builtins();
	void set_builtin_module(std::string& name, M_BaseObject* mod) { builtin_modules[name] = mod; }

	M_BaseObject* get_gateway_cache(M_BaseObject* key) { return gateway_cache.get(key); }

	virtual interpreter::BaseCompiler* get_compiler(vm::ThreadContext* context);
	virtual interpreter::PyFrame* create_frame(vm::ThreadContext* context, interpreter::Code* code, M_BaseObject* globals) { throw NotImplementedException("Abstract"); }

	virtual M_BaseObject* get_typeobject(interpreter::Typedef* def) { return typedef_cache->get(def); }
	virtual M_BaseObject* type(M_BaseObject* obj) { throw NotImplementedException("Abstract"); }

	virtual M_BaseObject* lookup(M_BaseObject* obj, const std::string& name) { throw NotImplementedException("Abstract"); }
	virtual M_BaseObject* lookup_type_cls(M_BaseObject* obj, const std::string& attr, M_BaseObject*& cls) { throw NotImplementedException("Abstract"); }

	virtual M_BaseObject* bool_type() { throw NotImplementedException("Abstract"); }
	virtual M_BaseObject* dict_type() { throw NotImplementedException("Abstract"); }
	virtual M_BaseObject* int_type() { throw NotImplementedException("Abstract"); }
	virtual M_BaseObject* object_type() { throw NotImplementedException("Abstract"); }
	virtual M_BaseObject* str_type() { throw NotImplementedException("Abstract"); }
	virtual M_BaseObject* tuple_type() { throw NotImplementedException("Abstract"); }
	virtual M_BaseObject* type_type() { throw NotImplementedException("Abstract"); }

	M_BaseObject* TypeError_type() { return type_TypeError; }
	M_BaseObject* StopIteration_type() { return type_StopIteration; }
	M_BaseObject* NameError_type() { return type_NameError; }
	M_BaseObject* UnboundLocalError_type() { return type_UnboundLocalError; }
	M_BaseObject* AttributeError_type() { return type_AttributeError; }
	bool match_exception(M_BaseObject* type1, M_BaseObject* type2) { return (type1 == type2); }

	virtual M_BaseObject* wrap(int x) { return wrap_int(x); }
	virtual M_BaseObject* wrap(const std::string& x) { return wrap_str(x); }
	virtual M_BaseObject* wrap(M_BaseObject* obj) { return obj; }

	virtual M_BaseObject* wrap_int(int x) { throw NotImplementedException("Abstract"); }
	virtual M_BaseObject* wrap_int(const std::string& x) { throw NotImplementedException("Abstract"); }

	virtual M_BaseObject* wrap_str(const std::string& x) { throw NotImplementedException("Abstract"); }

	virtual M_BaseObject* wrap_None() { throw NotImplementedException("Abstract"); }
	virtual M_BaseObject* wrap_True() { throw NotImplementedException("Abstract"); }
	virtual M_BaseObject* wrap_False() { throw NotImplementedException("Abstract"); }

	virtual M_BaseObject* new_bool(bool x) { if (x) return wrap_True(); else return wrap_False(); }
	virtual M_BaseObject* new_interned_str(const std::string& x);
	virtual M_BaseObject* new_tuple(std::vector<M_BaseObject*>& items) { throw NotImplementedException("Abstract"); }
	virtual M_BaseObject* new_dict() { throw NotImplementedException("Abstract"); }

	virtual int unwrap_int(M_BaseObject* obj, bool allow_conversion=true) { return obj->to_int(this, allow_conversion); }
	virtual std::string unwrap_str(M_BaseObject* obj) { return obj->to_string(this); }
	virtual void unwrap_tuple(M_BaseObject* obj, std::vector<M_BaseObject*>& list) { }

	M_BaseObject* get(M_BaseObject* descr, M_BaseObject* obj, M_BaseObject* type=nullptr);

	M_BaseObject* get_and_call_function(vm::ThreadContext* context, M_BaseObject* descr, const std::initializer_list<M_BaseObject*> args);
	M_BaseObject* call_args(vm::ThreadContext* context, M_BaseObject* func, interpreter::Arguments& args);
	M_BaseObject* call_obj_args(vm::ThreadContext* context, M_BaseObject* func, M_BaseObject* obj, interpreter::Arguments& args);
	M_BaseObject* call_function(vm::ThreadContext* context, M_BaseObject* func, const std::initializer_list<M_BaseObject*> args);

	M_BaseObject* getitem(M_BaseObject* obj, M_BaseObject* key);
	M_BaseObject* getitem_str(M_BaseObject* obj, const std::string& key);
	void setitem(M_BaseObject* obj, M_BaseObject* key, M_BaseObject* value);
	void setitem_str(M_BaseObject* obj, const std::string& key, M_BaseObject* value);

	M_BaseObject* getattr(M_BaseObject* obj, M_BaseObject* name);
	M_BaseObject* setattr(M_BaseObject* obj, M_BaseObject* name, M_BaseObject* value);
	M_BaseObject* delattr(M_BaseObject* obj, M_BaseObject* name);
	
	M_BaseObject* hash(M_BaseObject* obj);

	bool is_true(M_BaseObject* obj);
	bool i_is(M_BaseObject* obj1, M_BaseObject* obj2) { return (!obj2) ? false : obj2->i_is(this, obj1); }
	bool i_eq(M_BaseObject* obj1, M_BaseObject* obj2);
	std::size_t i_hash(M_BaseObject* obj);

	M_BaseObject* iter(M_BaseObject* obj);
	M_BaseObject* next(M_BaseObject* obj);

	M_BaseObject* lt(M_BaseObject* obj1, M_BaseObject* obj2);
	M_BaseObject* le(M_BaseObject* obj1, M_BaseObject* obj2);
	M_BaseObject* gt(M_BaseObject* obj1, M_BaseObject* obj2);
	M_BaseObject* ge(M_BaseObject* obj1, M_BaseObject* obj2);
	M_BaseObject* ne(M_BaseObject* obj1, M_BaseObject* obj2);
	M_BaseObject* eq(M_BaseObject* obj1, M_BaseObject* obj2);

	M_BaseObject* abs(M_BaseObject* obj);
	M_BaseObject* len(M_BaseObject* obj);

	M_BaseObject* pos(M_BaseObject* obj);
	M_BaseObject* neg(M_BaseObject* obj);
	M_BaseObject* not_(M_BaseObject* obj);
	M_BaseObject* invert(M_BaseObject* obj);

	M_BaseObject* add(M_BaseObject* obj1, M_BaseObject* obj2);
	M_BaseObject* sub(M_BaseObject* obj1, M_BaseObject* obj2);
	M_BaseObject* mul(M_BaseObject* obj1, M_BaseObject* obj2);
};

}
}

#endif
