#ifndef _BINOP_NODE_
#define _BINOP_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython {
namespace tree {

class BinOpNode : public ASTNode {
private:
    ASTNode *left, *right;
    mtpython::parse::BinaryOper op;

public:
    BinOpNode(const int line_nr);
    ~BinOpNode()
    {
        SAFE_DELETE(left);
        SAFE_DELETE(right);
    }

    ASTNode* get_left() { return this->left; }
    ASTNode* get_right() { return this->right; }
    void set_left(ASTNode* left) { this->left = left; }
    void set_right(ASTNode* right) { this->right = right; }
    mtpython::parse::BinaryOper get_op() { return this->op; }
    void set_op(const mtpython::parse::BinaryOper op) { this->op = op; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": BinOp: " << std::endl;
        std::cout << blank << "  " << line << ": Operator: " << binop2str(op)
                  << std::endl;
        std::cout << blank << "  " << line << ": Left: " << std::endl;
        left->print(padding + 4);
        std::cout << blank << "  " << line << ": Right: " << std::endl;
        right->print(padding + 4);
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_binop(this); }
    virtual NodeType get_tag() { return NT_BINOP; }
};

} // namespace tree
} // namespace mtpython

#endif /* _BINOP_NODE_ */
