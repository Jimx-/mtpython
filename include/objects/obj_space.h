#ifndef _OBJ_SPACE_H_
#define _OBJ_SPACE_H_

#include <string>
#include <initializer_list>
#include <unordered_map>
#include "objects/base_object.h"
#include "interpreter/arguments.h"
#include "objects/space_cache.h"

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
	virtual interpreter::PyFrame* create_frame(vm::ThreadContext* context, interpreter::Code* code, M_BaseObject* globals) { return nullptr; }

	virtual M_BaseObject* get_typeobject(interpreter::Typedef* def) { return typedef_cache->get(def); }
	virtual M_BaseObject* type(M_BaseObject* obj) { return nullptr; }

	virtual M_BaseObject* lookup(M_BaseObject* obj, std::string& name) { return nullptr; }
	virtual M_BaseObject* lookup_type_cls(M_BaseObject* obj, std::string& attr, M_BaseObject*& cls) { return nullptr; }

	virtual M_BaseObject* bool_type() { return nullptr; }
	virtual M_BaseObject* dict_type() { return nullptr; }
	virtual M_BaseObject* int_type() { return nullptr; }
	virtual M_BaseObject* object_type() { return nullptr; }
	virtual M_BaseObject* str_type() { return nullptr; }
	virtual M_BaseObject* tuple_type() { return nullptr; }
	virtual M_BaseObject* type_type() { return nullptr; }

	virtual M_BaseObject* wrap(int x) { return wrap_int(x); }
	virtual M_BaseObject* wrap(std::string& x) { return wrap_str(x); }
	virtual M_BaseObject* wrap(M_BaseObject* obj) { return obj; }

	virtual M_BaseObject* wrap_int(int x) { return nullptr; }
	virtual M_BaseObject* wrap_int(std::string& x) { return nullptr; }

	virtual M_BaseObject* wrap_str(std::string& x) { return nullptr; }

	virtual M_BaseObject* wrap_None() { return nullptr; }
	virtual M_BaseObject* wrap_True() { return nullptr; }
	virtual M_BaseObject* wrap_False() { return nullptr; }

	virtual M_BaseObject* new_bool(bool x) { if (x) return wrap_True(); else return wrap_False(); }
	virtual M_BaseObject* new_interned_str(std::string& x);
	virtual M_BaseObject* new_tuple(std::vector<M_BaseObject*>& items) { return nullptr; }
	virtual M_BaseObject* new_dict() { return nullptr; }

	virtual int unwrap_int(M_BaseObject* obj, bool allow_conversion=true) { return obj->to_int(this, allow_conversion); }
	virtual std::string unwrap_str(M_BaseObject* obj) { return obj->to_string(this); }
	virtual void unwrap_tuple(M_BaseObject* obj, std::vector<M_BaseObject*>& list) { }

	virtual M_BaseObject* get_and_call_function(vm::ThreadContext* context, M_BaseObject* descr, std::initializer_list<M_BaseObject*> args);
	virtual M_BaseObject* call_args(vm::ThreadContext* context, M_BaseObject* func, interpreter::Arguments& args);
	virtual M_BaseObject* call_function(vm::ThreadContext* context, M_BaseObject* func, std::initializer_list<M_BaseObject*> args);

	virtual M_BaseObject* getitem(M_BaseObject* obj, M_BaseObject* key);
	virtual M_BaseObject* getitem_str(M_BaseObject* obj, std::string& key);
	virtual void setitem(M_BaseObject* obj, M_BaseObject* key, M_BaseObject* value);
	
	virtual M_BaseObject* hash(M_BaseObject* obj);

	virtual bool is_true(M_BaseObject* obj);
	virtual bool i_is(M_BaseObject* obj1, M_BaseObject* obj2) { return (!obj2) ? false : obj2->i_is(this, obj1); }
	virtual bool i_eq(M_BaseObject* obj1, M_BaseObject* obj2);
	virtual std::size_t i_hash(M_BaseObject* obj);

	virtual M_BaseObject* eq(M_BaseObject* obj1, M_BaseObject* obj2);

	virtual M_BaseObject* add(M_BaseObject* obj1, M_BaseObject* obj2);
};

}
}

#endif
