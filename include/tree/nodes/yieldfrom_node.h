#ifndef _YIELDFROM_NODE_
#define _YIELDFROM_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython {
namespace tree {

class YieldFromNode : public ASTNode {
private:
    ASTNode* value;

public:
    YieldFromNode(const int line_nr);
    ~YieldFromNode() { SAFE_DELETE(value); }

    ASTNode* get_value() { return value; }
    void set_value(ASTNode* value) { this->value = value; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": YieldFrom: " << std::endl;
        std::cout << blank << "  " << line << ": Value: " << std::endl;
        if (value) value->print(padding + 4);
    }

    virtual NodeType get_tag() { return NT_YIELDFROM; }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_yieldfrom(this); }
};

} // namespace tree
} // namespace mtpython

#endif /* _YIELDFROM_NODE_ */
