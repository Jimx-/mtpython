#include "parse/parser.h"
#include "parse/symtable.h"
#include "parse/codegen.h"
#include "interpreter/compiler.h"

using namespace mtpython::interpreter;
using namespace mtpython::parse;

PyCompiler::PyCompiler(mtpython::vm::ThreadContext* context) : BaseCompiler(context)
{

}

Code* PyCompiler::compile(const std::string& source, const std::string& filename, const std::string& mode, int flags)
{
	mtpython::parse::SourceType type = mtpython::parse::SourceType::ST_ERROR;

	if (mode == "exec") type = mtpython::parse::SourceType::ST_FILE_INPUT;

	flags = flags | PyCF_SOURCE_IS_UTF8;

	CompileInfo info(filename, type, flags);

	Parser parser(space, source, &info, flags);

	mtpython::tree::ASTNode* module = parser.parse();
	//module->print(0);
	SymtableVisitor symtab(space, module);
	ModuleCodeGenerator codegen(space, module, &symtab, &info);
	mtpython::interpreter::PyCode* code = codegen.build();

	SAFE_DELETE(module);

	return code;
}
