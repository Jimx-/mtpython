#include "tree/nodes.h"

using namespace mtpython::tree;

#define NODE_CONSTRUCTOR(name) name::name(int line_nr) : ASTNode(line_nr)

NODE_CONSTRUCTOR(ModuleNode)
{
	body = NULL;
}

NODE_CONSTRUCTOR(FunctionDefNode)
{
	body = NULL;
}

NODE_CONSTRUCTOR(NameNode)
{
	ctx = EC_LOAD;
}

NODE_CONSTRUCTOR(ArgumentsNode)
{

}

NODE_CONSTRUCTOR(ReturnNode)
{

}

NODE_CONSTRUCTOR(BinOpNode)
{
	left = right = NULL;
}

NODE_CONSTRUCTOR(UnaryOpNode)
{
	operand = NULL;
}

NODE_CONSTRUCTOR(CompareNode)
{
	left = NULL;
}

NODE_CONSTRUCTOR(IfExpNode)
{
	test = NULL;
	body = NULL;
	orelse = NULL;
}

NODE_CONSTRUCTOR(TupleNode)
{
	ctx = EC_LOAD;
	elts.clear();
}

NODE_CONSTRUCTOR(AssignNode)
{
	value = NULL;
	targets.clear();
}

NODE_CONSTRUCTOR(AugAssignNode)
{
	value = NULL;
	target = NULL;
}

NODE_CONSTRUCTOR(BreakNode)
{

}

NODE_CONSTRUCTOR(ContinueNode)
{

}

NODE_CONSTRUCTOR(PassNode)
{

}

NODE_CONSTRUCTOR(DeleteNode)
{
	targets.clear();
}

NODE_CONSTRUCTOR(YieldNode)
{
	value = NULL;
}

NODE_CONSTRUCTOR(YieldFromNode)
{
	value = NULL;
}

NODE_CONSTRUCTOR(RaiseNode)
{
	exc = NULL;
	cause = NULL;
}

NODE_CONSTRUCTOR(IfNode)
{
	test = NULL;
	body = NULL;
	orelse = NULL;
}

NODE_CONSTRUCTOR(ForNode)
{
	target = NULL;
	iter = NULL;
	body = NULL;
	orelse = NULL;
}

NODE_CONSTRUCTOR(WhileNode)
{
	test = NULL;
	body = NULL;
	orelse = NULL;
}
