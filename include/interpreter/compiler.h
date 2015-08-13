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

	virtual M_Code* compile(std::string& source, std::string& filename, mtpython::parse::SourceType type, int flags) { return nullptr; }
};

class PyCompiler : public BaseCompiler {
public:
	PyCompiler(vm::ThreadContext* context);
	virtual M_Code* compile(std::string& source, std::string& filename, mtpython::parse::SourceType type, int flags);
};

}
}

#endif
