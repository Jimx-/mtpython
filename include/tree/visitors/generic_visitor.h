#ifndef _TREE_GENERIC_VISITOR_H_
#define _TREE_GENERIC_VISITOR_H_

#include "tree/nodes.h"

namespace mtpython { 
namespace tree {

class GenericVisitor : public ASTVisitor {
public:

	virtual ASTNode* visit_module(ModuleNode* node) 
	{
		visit_sequence(node->get_body());

		return node;
	}

	virtual ASTNode* visit_attribute(AttributeNode* node)
	{
		node->get_value()->visit(this);
	}

	virtual ASTNode* visit_arguments(ArgumentsNode* node) 
	{
		std::vector<ASTNode*>& args = node->get_args();
		for (unsigned int i = 0; i < args.size(); i++) {
			args[i]->visit(this);
		}

		return node;
	}
	virtual ASTNode* visit_assign(AssignNode* node) 
	{
		std::vector<ASTNode*>& targets = node->get_targets();
		for (unsigned int i = 0; i < targets.size(); i++) {
			targets[i]->visit(this);
		}
		node->get_value()->visit(this);
		
		return node;
	}
	
	virtual ASTNode* visit_augassign(AugAssignNode* node) 
	{
		node->get_target()->visit(this);
		node->get_value()->visit(this);
		
		return node;
	}
	
	virtual ASTNode* visit_binop(BinOpNode* node) 
	{
		node->get_left()->visit(this);
		node->get_right()->visit(this);
		
		return node; 
	}

	virtual ASTNode* visit_break(BreakNode* node) 
	{
		return node; 
	}

	virtual ASTNode* visit_call(CallNode* node)
	{
		node->get_func()->visit(this);
		std::vector<ASTNode*>& args = node->get_args();
		for (unsigned int i = 0; i < args.size(); i++) {
			args[i]->visit(this);
		}
		std::vector<KeywordNode*>& keywords = node->get_keywords();
		for (unsigned int i = 0; i < keywords.size(); i++) {
			keywords[i]->visit(this);
		}
		return node;
	}

	virtual ASTNode* visit_compare(CompareNode* node) { return node; }

	virtual ASTNode* visit_const(ConstNode* node) { return node; }
	
	virtual ASTNode* visit_continue(ContinueNode* node) 
	{
		return node; 
	}

	virtual ASTNode* visit_delete(DeleteNode* node) 
	{
		std::vector<ASTNode*>& targets = node->get_targets();
		for (unsigned int i = 0; i < targets.size(); i++) {
			targets[i]->visit(this);
		}
		return node; 
	}

	virtual ASTNode* visit_excepthandler(ExceptHandlerNode* node)
	{
		node->get_type()->visit(this);
		visit_sequence(node->get_body());

		return node;
	}

	virtual ASTNode* visit_expr(ExprNode* node)
	{
		node->get_value()->visit(this);

		return node;
	}

	virtual ASTNode* visit_for(ForNode* node) 
	{
		node->get_target()->visit(this);
		node->get_iter()->visit(this);
		visit_sequence(node->get_body());
		if (ASTNode* orelse = node->get_orelse())
			visit_sequence(orelse);
		
		return node; 
	}

	virtual ASTNode* visit_functiondef(FunctionDefNode* node) 
	{
		return node; 
	}

	virtual ASTNode* visit_if(IfNode* node) 
	{
		node->get_test()->visit(this);
		visit_sequence(node->get_body());
		if (ASTNode* orelse = node->get_orelse())
			visit_sequence(orelse);
		
		return node; 
	}
	
	virtual ASTNode* visit_ifexp(IfExpNode* node) {return node; }
	
	virtual ASTNode* visit_keyword(KeywordNode* node) { return node; }

	virtual ASTNode* visit_name(NameNode* node) 
	{
		return node; 
	}

	virtual ASTNode* visit_number(NumberNode* node) 
	{
		return node; 
	}

	virtual ASTNode* visit_string(StringNode* node)
	{
		return node;
	}

	virtual ASTNode* visit_pass(PassNode* node) 
	{ 
		return node;
	}

	virtual ASTNode* visit_raise(RaiseNode* node) {return node; }
	virtual ASTNode* visit_return(ReturnNode* node) { return node;}

	virtual ASTNode* visit_try(TryNode* node)
	{
		visit_sequence(node->get_body());
		std::vector<ExceptHandlerNode*>& excepthandlers = node->get_handlers();
		for (unsigned int i = 0; i < excepthandlers.size(); i++) {
			excepthandlers[i]->visit(this);
		}
		visit_sequence(node->get_orelse());
		visit_sequence(node->get_finalbody());

		return node;
	}

	virtual ASTNode* visit_tuple(TupleNode* node) {return node; }
	virtual ASTNode* visit_unaryop(UnaryOpNode* node) { return node;}

	virtual ASTNode* visit_while(WhileNode* node)
	{
		node->get_test()->visit(this);
		visit_sequence(node->get_body());
		if (ASTNode* orelse = node->get_orelse())
			visit_sequence(orelse);

		return node;
	}

	virtual ASTNode* visit_yield(YieldNode* node) {return node; }
	virtual ASTNode* visit_yieldfrom(YieldFromNode* node) { return node;}
};

}
}

#endif /* _TREE_GENERIC_VISITOR_H_ */
