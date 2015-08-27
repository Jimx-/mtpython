#include "tree/nodes.h"

using namespace mtpython::tree;

#define NODE_CONSTRUCTOR(name) name::name(int line_nr) : ASTNode(line_nr)

NODE_CONSTRUCTOR(ModuleNode)
{
	body = nullptr;
}

NODE_CONSTRUCTOR(FunctionDefNode)
{
	body = nullptr;
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
	left = right = nullptr;
}

NODE_CONSTRUCTOR(UnaryOpNode)
{
	operand = nullptr;
}

NODE_CONSTRUCTOR(CompareNode)
{
	left = nullptr;
}

NODE_CONSTRUCTOR(IfExpNode)
{
	test = nullptr;
	body = nullptr;
	orelse = nullptr;
}

NODE_CONSTRUCTOR(TupleNode)
{
	ctx = EC_LOAD;
	elts.clear();
}

NODE_CONSTRUCTOR(AssignNode)
{
	value = nullptr;
	targets.clear();
}

NODE_CONSTRUCTOR(AugAssignNode)
{
	value = nullptr;
	target = nullptr;
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
	value = nullptr;
}

NODE_CONSTRUCTOR(YieldFromNode)
{
	value = nullptr;
}

NODE_CONSTRUCTOR(RaiseNode)
{
	exc = nullptr;
	cause = nullptr;
}

NODE_CONSTRUCTOR(IfNode)
{
	test = nullptr;
	body = nullptr;
	orelse = nullptr;
}

NODE_CONSTRUCTOR(ForNode)
{
	target = nullptr;
	iter = nullptr;
	body = nullptr;
	orelse = nullptr;
}

NODE_CONSTRUCTOR(WhileNode)
{
	test = nullptr;
	body = nullptr;
	orelse = nullptr;
}

NODE_CONSTRUCTOR(NumberNode)
{
	value = nullptr;
}

NODE_CONSTRUCTOR(StringNode)
{
	value = nullptr;
}

NODE_CONSTRUCTOR(CallNode)
{
	func = nullptr;
}

NODE_CONSTRUCTOR(KeywordNode)
{
	value = nullptr;
}

NODE_CONSTRUCTOR(ExprNode)
{
	value = nullptr;
}

NODE_CONSTRUCTOR(ConstNode)
{
	value = nullptr;
}
