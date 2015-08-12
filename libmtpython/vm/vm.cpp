#include "parse/parser.h"
#include "parse/symtable.h"
#include "parse/codegen.h"
#include "interpreter/pycode.h"
#include "vm/vm.h"

using namespace mtpython::vm;
using namespace mtpython::objects;
using namespace mtpython::parse;

PyVM::PyVM(ObjSpace* space)
{
	this->space = space;
	space->set_vm(this);
}

void PyVM::run_file(std::string& filename)
{
	Parser parser(space, filename);

	mtpython::tree::ASTNode* module = parser.parse();
	SymtableVisitor symtab(space, module);
	ModuleCodeGenerator codegen(space, module, &symtab);
	mtpython::interpreter::PyCode* code = codegen.build();

	while(1);
}
