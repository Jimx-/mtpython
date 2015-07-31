#include "tree/nodes.h"

using namespace mtpython::tree;

#define NODE_CONSTRUCTOR(name) name::name(int line_nr) : ASTNode(line_nr)

NODE_CONSTRUCTOR(ModuleNode)
{
	body = NULL;
}
