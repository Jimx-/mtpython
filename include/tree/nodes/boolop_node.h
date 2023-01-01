#ifndef _BOOLOP_NODE_
#define _BOOLOP_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"
#include <vector>

namespace mtpython {
namespace tree {

class BoolOpNode : public ASTNode {
private:
    std::vector<ASTNode*> values;
    mtpython::parse::BinaryOper op;

public:
    BoolOpNode(const int line_nr);
    ~BoolOpNode()
    {
        for (auto& p : values)
            delete p;
    }

    std::vector<ASTNode*>& get_values() { return this->values; }
    void push_value(ASTNode* value) { values.push_back(value); }
    mtpython::parse::BinaryOper get_op() { return this->op; }
    void set_op(const mtpython::parse::BinaryOper op) { this->op = op; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": BoolOp: " << std::endl;
        std::cout << blank << "  " << line << ": Operator: " << binop2str(op)
                  << std::endl;
        std::cout << blank << "  " << line << ": Values: " << std::endl;
        for (const auto& p : values)
            p->print(padding + 4);
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_boolop(this); }
    virtual NodeType get_tag() { return NT_BINOP; }
};

} // namespace tree
} // namespace mtpython

#endif /* _BINOP_NODE_ */
