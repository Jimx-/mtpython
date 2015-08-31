#ifndef _INTERPRETER_COMPILER_H_
#define _INTERPRETER_COMPILER_H_

#include "interpreter/code.h"
#include "utils/source_buffer.h"
#include "objects/obj_space.h"
#include "vm/vm.h"
#include <string>

namespace mtpython {
namespace interpreter {

class BaseCompiler {
protected:
	mtpython::objects::ObjSpace* space;
public:
	BaseCompiler(vm::ThreadContext* context) : space(context->get_space()) { }

	virtual Code* compile(const std::string& source, const std::string& filename, const std::string& mode, int flags) { return nullptr; }
};

class PyCompiler : public BaseCompiler {
public:
	PyCompiler(vm::ThreadContext* context);
	virtual Code* compile(const std::string& source, const std::string& filename, const std::string& mode, int flags);
};

}
}

#endif
