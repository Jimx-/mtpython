#ifndef _EXPR_NODE_
#define _EXPR_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include "objects/base_object.h"
#include <iostream>
#include "macros.h"

namespace mtpython {
namespace tree {

class ExprNode : public ASTNode {
private:
    ASTNode* value;

public:
    ExprNode(const int line_nr);
    ~ExprNode() { SAFE_DELETE(value); }

    ASTNode* get_value() { return value; }
    void set_value(ASTNode* value) { this->value = value; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Expr: " << std::endl;
        value->print(padding + 4);
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_expr(this); }
    virtual NodeType get_tag() { return NT_EXPR; }
};

} // namespace tree
} // namespace mtpython

#endif /* _EXPR_NODE_ */
