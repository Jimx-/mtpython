#ifndef _OBJ_SPACE_H_
#define _OBJ_SPACE_H_

#include <string>
#include <initializer_list>
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

	TypedefCache* typedef_cache;
	GatewayCache gateway_cache;

	M_BaseObject* execute_binop(vm::ThreadContext* context, M_BaseObject* impl, M_BaseObject* left, M_BaseObject* right);
public:
	ObjSpace() : gateway_cache(this) { }

	void set_vm(mtpython::vm::PyVM* vm) { this->vm = vm; }

	M_BaseObject* get_gateway_cache(M_BaseObject* key) { return gateway_cache.get(key); }

	virtual interpreter::BaseCompiler* get_compiler(vm::ThreadContext* context);
	virtual interpreter::PyFrame* create_frame(vm::ThreadContext* context, interpreter::Code* code, M_BaseObject* globals) { return nullptr; }

	virtual M_BaseObject* get_typeobject(interpreter::Typedef* def) { return typedef_cache->get(def); }
	virtual M_BaseObject* type(M_BaseObject* obj) { return nullptr; }

	virtual M_BaseObject* lookup_type_cls(M_BaseObject* obj, std::string& attr, M_BaseObject*& cls) { return nullptr; }

	virtual M_BaseObject* wrap(int x) { return wrap_int(x); }
	virtual M_BaseObject* wrap(std::string& x) { return wrap_str(x); }
	virtual M_BaseObject* wrap(M_BaseObject* obj) { return obj; }

	virtual M_BaseObject* wrap_int(int x) { return nullptr; }
	virtual M_BaseObject* wrap_int(std::string& x) { return nullptr; }

	virtual M_BaseObject* wrap_str(std::string& x) { return nullptr; }

	virtual int unwrap_int(M_BaseObject* obj, bool allow_conversion=true) { return obj->to_int(this, allow_conversion); }

	virtual M_BaseObject* call_args(vm::ThreadContext* context, M_BaseObject* func, interpreter::Arguments& args);
	virtual M_BaseObject* call_function(vm::ThreadContext* context, M_BaseObject* func, std::initializer_list<M_BaseObject*> args);

	virtual M_BaseObject* add(vm::ThreadContext* context, M_BaseObject* obj1, M_BaseObject* obj2);
};

}
}

#endif
