#ifndef _SUBSCRIPT_NODE_
#define _SUBSCRIPT_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython {
namespace tree {

class SubscriptNode : public ASTNode {
private:
    ASTNode* value;
    ASTNode* slice;
    ExprContext ctx;

public:
    SubscriptNode(const int line_nr);
    ~SubscriptNode() {}

    ASTNode* get_value() { return value; }
    void set_value(ASTNode* value) { this->value = value; }
    ASTNode* get_slice() { return slice; }
    void set_slice(ASTNode* slice) { this->slice = slice; }
    ExprContext get_context() { return ctx; }
    virtual void set_context(ExprContext ctx) { this->ctx = ctx; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Subscript: " << std::endl;
        std::cout << blank << "  " << line << ": Value: " << std::endl;
        value->print(padding + 4);
        std::cout << blank << "  " << line << ": Slice: " << std::endl;
        slice->print(padding + 4);
        std::cout << blank << "  " << line << ": Context: " << exprctx2str(ctx)
                  << std::endl;
    }

    virtual NodeType get_tag() { return NT_SUBSCRIPT; }
    virtual void visit(ASTVisitor* visitor) { visitor->visit_subscript(this); }
};

} // namespace tree
} // namespace mtpython

#endif /* _SUBSCRIPT_NODE_ */
