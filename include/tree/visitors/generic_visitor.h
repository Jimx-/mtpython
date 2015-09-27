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

	virtual ASTNode* visit_alias(AliasNode* node)
	{
		return node;
	}

	virtual ASTNode* visit_attribute(AttributeNode* node)
	{
		node->get_value()->visit(this);
		return node;
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

	virtual ASTNode* visit_classdef(ClassDefNode* node)
	{
		visit_sequence(node->get_body());
		std::vector<ASTNode*> bases = node->get_bases();
		for(std::size_t i = 0; i < bases.size(); i++) bases[i]->visit(this);
		return node;
	}

	virtual ASTNode* visit_compare(CompareNode* node) { return node; }

	virtual ASTNode* visit_comprehension(ComprehensionNode* node)
	{
		node->get_target()->visit(this);
		node->get_iter()->visit(this);

		std::vector<ASTNode*>& ifs = node->get_ifs();
		for (unsigned int i = 0; i < ifs.size(); i++) {
			ifs[i]->visit(this);
		}

		return node;
	}

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

	virtual ASTNode* visit_dict(DictNode* node)
	{
		std::vector<ASTNode*>& keys = node->get_keys();
		for (unsigned int i = 0; i < keys.size(); i++) {
			keys[i]->visit(this);
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
		node->get_args()->visit(this);
		visit_sequence(node->get_body());
		return node;
	}

	virtual  ASTNode* visit_generatorexp(GeneratorExpNode* node)
	{
		node->get_elt()->visit(this);
		std::vector<ComprehensionNode*>& comprehensions = node->get_comprehensions();
		for (unsigned int i = 0; i < comprehensions.size(); i++) {
			comprehensions[i]->visit(this);
		}

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

	virtual ASTNode* visit_import(ImportNode* node)
	{
		std::vector<AliasNode*>& names = node->get_names();
		for (unsigned int i = 0; i < names.size(); i++) {
			names[i]->visit(this);
		}

		return node;
	}

	virtual ASTNode* visit_importfrom(ImportFromNode* node)
	{
		std::vector<AliasNode*>& names = node->get_names();
		for (unsigned int i = 0; i < names.size(); i++) {
			names[i]->visit(this);
		}

		return node;
	}

	virtual ASTNode* visit_index(IndexNode* node)
	{
		node->get_value()->visit(this);

		return node;
	}

	virtual ASTNode* visit_keyword(KeywordNode* node) { return node; }

	virtual ASTNode* visit_lambda(LambdaNode* node)
	{
		node->get_args()->visit(this);
		visit_sequence(node->get_body());
		return node;
	}

	virtual ASTNode* visit_list(ListNode* node)
	{
		std::vector<ASTNode*>& elements = node->get_elements();
		for (unsigned int i = 0; i < elements.size(); i++) {
			elements[i]->visit(this);
		}

		return node;
	}

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

	virtual ASTNode* visit_set(SetNode* node)
	{
		std::vector<ASTNode*>& elements = node->get_elements();
		for (unsigned int i = 0; i < elements.size(); i++) {
			elements[i]->visit(this);
		}

		return node;
	}

	virtual ASTNode* visit_slice(SliceNode* node)
	{
		ASTNode* tmp;
		tmp = node->get_lower();
		if (tmp) tmp->visit(this);
		tmp = node->get_upper();
		if (tmp) tmp->visit(this);
		tmp = node->get_step();
		if (tmp) tmp->visit(this);

		return node;
	}

	virtual ASTNode* visit_starred(StarredNode* node)
	{
		node->get_value()->visit(this);
	}

	virtual ASTNode* visit_subscript(SubscriptNode* node)
	{
		node->get_value()->visit(this);
		node->get_slice()->visit(this);

		return node;
	}

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

	virtual ASTNode* visit_tuple(TupleNode* node)
	{
		std::vector<ASTNode*>& elements = node->get_elements();
		for (unsigned int i = 0; i < elements.size(); i++) {
			elements[i]->visit(this);
		}

		return node;
	}

	virtual ASTNode* visit_unaryop(UnaryOpNode* node) { return node;}

	virtual ASTNode* visit_while(WhileNode* node)
	{
		node->get_test()->visit(this);
		visit_sequence(node->get_body());
		if (ASTNode* orelse = node->get_orelse())
			visit_sequence(orelse);

		return node;
	}

	virtual ASTNode* visit_with(WithNode* node)
	{
		std::vector<WithItemNode*> items = node->get_items();
		for (unsigned int i = 0; i < items.size(); i++) {
			items[i]->visit(this);
		}
		visit_sequence(node->get_body());

		return node;
	}

	virtual  ASTNode* visit_withitem(WithItemNode* node)
	{
		node->get_context_expr()->visit(this);
		ASTNode* opt_vars = node->get_optional_vars();
		if (opt_vars) opt_vars->visit(this);

		return node;
	}

	virtual ASTNode* visit_yield(YieldNode* node) {return node; }
	virtual ASTNode* visit_yieldfrom(YieldFromNode* node) { return node;}
};

}
}

#endif /* _TREE_GENERIC_VISITOR_H_ */
