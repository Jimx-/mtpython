#ifndef _CONST_NODE_
#define _CONST_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include "objects/base_object.h"
#include <iostream>
#include "macros.h"

namespace mtpython {
namespace tree {

class ConstNode : public ASTNode {
private:
    mtpython::objects::M_BaseObject* value;

public:
    ConstNode(const int line_nr);
    ~ConstNode() {}

    mtpython::objects::M_BaseObject* get_value() { return value; }
    void set_value(mtpython::objects::M_BaseObject* value)
    {
        this->value = value;
    }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Const: ";
        value->dbg_print();
        std::cout << std::endl;
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_const(this); }
    virtual NodeType get_tag() { return NT_CONST; }
};

} // namespace tree
} // namespace mtpython

#endif /* _CONST_NODE_ */
