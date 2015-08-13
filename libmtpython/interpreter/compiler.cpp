#include "parse/parser.h"
#include "parse/symtable.h"
#include "parse/codegen.h"
#include "interpreter/pycode.h"
#include "parse/compile_info.h"
#include "interpreter/compiler.h"

using namespace mtpython::interpreter;
using namespace mtpython::parse;

PyCompiler::PyCompiler(mtpython::vm::ThreadContext* context) : BaseCompiler(context)
{

}

M_Code* PyCompiler::compile(std::string& source, std::string& filename, mtpython::parse::SourceType type, int flags)
{
	CompileInfo info(filename, type, flags);

	Parser parser(space, source, &info);

	mtpython::tree::ASTNode* module = parser.parse();
	SymtableVisitor symtab(space, module);
	ModuleCodeGenerator codegen(space, module, &symtab);
	mtpython::interpreter::PyCode* code = codegen.build();

	return code;
}
