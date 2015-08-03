#include "parse/parser.h"
#include "tree/print_visitor.h"
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

	parser.parse()->visit(new mtpython::tree::PrintVisitor());
	//parser.parse()->print(0);

	while(1);
}
