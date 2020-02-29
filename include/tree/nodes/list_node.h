#ifndef _LIST_NODE_
#define _LIST_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython {
namespace tree {

class ListNode : public ASTNode {
private:
    std::vector<ASTNode*> elts;
    ExprContext ctx;

public:
    ListNode(const int line_nr);
    ~ListNode()
    {
        for (std::size_t i = 0; i < elts.size(); i++)
            SAFE_DELETE(elts[i]);
    }

    void push_element(ASTNode* elt) { elts.push_back(elt); }
    std::vector<ASTNode*>& get_elements() { return elts; }
    ExprContext get_context() { return ctx; }
    virtual void set_context(ExprContext ctx)
    {
        this->ctx = ctx;
        for (unsigned int i = 0; i < elts.size(); i++)
            elts[i]->set_context(ctx);
    }

    virtual NodeType get_tag() { return NT_LIST; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": List:" << std::endl;
        std::cout << blank << "  " << line << ": Elements:" << std::endl;
        for (unsigned int i = 0; i < elts.size(); i++)
            elts[i]->print(padding + 4);
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_list(this); }
};

} // namespace tree
} // namespace mtpython

#endif /* _LIST_NODE_ */
