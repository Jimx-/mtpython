#ifndef _TREE_PRINT_VISITOR_H_
#define _TREE_PRINT_VISITOR_H_

#include <iostream>
#include "tree/nodes.h"

namespace mtpython {
namespace tree {

class PrintVisitor : public ASTVisitor {
public:
    virtual void visit_sequence(ASTNode* seq)
    {
        ASTNode* node = seq;
        std::cout << "[";
        if (node) node->visit(this);
        node = node->get_sibling();
        while (node) {
            std::cout << ", ";
            node->visit(this);
            node = node->get_sibling();
        }
        std::cout << "]";
    }

    virtual ASTNode* visit_module(ModuleNode* node)
    {
        std::cout << "Module(body=";
        visit_sequence(node->get_body());
        std::cout << ")";

        return node;
    }

    virtual ASTNode* visit_arguments(ArgumentsNode* node)
    {
        std::cout << "Arguments(args=[";
        std::vector<ASTNode*>& args = node->get_args();
        for (unsigned int i = 0; i < args.size(); i++) {
            if (i > 0) std::cout << ",";
            args[i]->visit(this);
        }
        std::cout << "])";

        return node;
    }
    virtual ASTNode* visit_assign(AssignNode* node)
    {
        std::cout << "Assign(target=[";
        std::vector<ASTNode*>& targets = node->get_targets();
        for (unsigned int i = 0; i < targets.size(); i++) {
            if (i > 0) std::cout << ",";
            targets[i]->visit(this);
        }
        std::cout << "], value=";
        node->get_value()->visit(this);
        std::cout << ")";

        return node;
    }

    virtual ASTNode* visit_augassign(AugAssignNode* node)
    {
        std::cout << "AugAssign(target=";
        node->get_target()->visit(this);
        std::cout << ", op=" << binop2str(node->get_op()) << ", value=";
        node->get_value()->visit(this);
        std::cout << ")";

        return node;
    }

    virtual ASTNode* visit_binop(BinOpNode* node)
    {
        std::cout << "BinOp(left=";
        node->get_left()->visit(this);
        std::cout << ", op=" << binop2str(node->get_op()) << ", right=";
        node->get_right()->visit(this);
        std::cout << ")";

        return node;
    }

    virtual ASTNode* visit_break(BreakNode* node)
    {
        std::cout << "Break()";
        return node;
    }

    virtual ASTNode* visit_compare(CompareNode* node) { return node; }

    virtual ASTNode* visit_continue(ContinueNode* node)
    {
        std::cout << "Continue()";
        return node;
    }

    virtual ASTNode* visit_delete(DeleteNode* node)
    {
        std::cout << "Delete(targets=[";
        std::vector<ASTNode*>& targets = node->get_targets();
        for (unsigned int i = 0; i < targets.size(); i++) {
            if (i > 0) std::cout << ",";
            targets[i]->visit(this);
        }
        std::cout << "])";
        return node;
    }

    virtual ASTNode* visit_for(ForNode* node)
    {
        std::cout << "For(target=";
        node->get_target()->visit(this);
        std::cout << ", iter=";
        node->get_iter()->visit(this);
        std::cout << ", body=";
        node->get_body()->visit(this);
        std::cout << ", orelse=";
        node->get_orelse()->visit(this);
        std::cout << ")";

        return node;
    }

    virtual ASTNode* visit_functiondef(FunctionDefNode* node) { return node; }

    virtual ASTNode* visit_if(IfNode* node) { return node; }

    virtual ASTNode* visit_ifexp(IfExpNode* node) { return node; }

    virtual ASTNode* visit_name(NameNode* node)
    {
        std::cout << "Name(id=\"" << node->get_name()
                  << ", ctx=" << exprctx2str(node->get_context()) << ")";
        return node;
    }

    virtual ASTNode* visit_number(NumberNode* node)
    {
        std::cout << "Num(value=";
        node->get_value()->dbg_print();
        std::cout << ")";
        return node;
    }

    virtual ASTNode* visit_pass(PassNode* node)
    {
        std::cout << "Pass()";
        return node;
    }

    virtual ASTNode* visit_raise(RaiseNode* node) { return node; }
    virtual ASTNode* visit_return(ReturnNode* node) { return node; }
    virtual ASTNode* visit_tuple(TupleNode* node) { return node; }
    virtual ASTNode* visit_unaryop(UnaryOpNode* node) { return node; }
    virtual ASTNode* visit_while(WhileNode* node) { return node; }
    virtual ASTNode* visit_yield(YieldNode* node) { return node; }
    virtual ASTNode* visit_yieldfrom(YieldFromNode* node) { return node; }
};

} // namespace tree
} // namespace mtpython

#endif /* _TREE_PRINT_VISITOR_H_ */
