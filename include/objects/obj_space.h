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
public:
	ObjSpace() : gateway_cache(this) { }

	void set_vm(mtpython::vm::PyVM* vm) { this->vm = vm; }

	M_BaseObject* get_gateway_cache(M_BaseObject* key) { return gateway_cache.get(key); }

	virtual interpreter::BaseCompiler* get_compiler(vm::ThreadContext* context);

	virtual M_BaseObject* get_typeobject(interpreter::Typedef* def) { return typedef_cache->get(def); }

	virtual M_BaseObject* wrap(int x) { return wrap_int(x); }
	virtual M_BaseObject* wrap(M_BaseObject* obj) { return obj; }

	virtual M_BaseObject* wrap_int(int x) { return nullptr; }
	virtual M_BaseObject* wrap_int(std::string& x) { return nullptr; }

	virtual M_BaseObject* call_args(vm::ThreadContext* context, M_BaseObject* func, interpreter::Arguments& args);
	virtual M_BaseObject* call_function(vm::ThreadContext* context, M_BaseObject* func, std::initializer_list<M_BaseObject*> args);
};

}
}

#endif
