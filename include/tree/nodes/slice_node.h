#ifndef _SLICE_NODE_
#define _SLICE_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include "objects/base_object.h"
#include <iostream>
#include "macros.h"

namespace mtpython {
namespace tree {

class IndexNode : public ASTNode {
private:
    ASTNode* value;
public:
    IndexNode(const int line_nr);
    ~IndexNode() { SAFE_DELETE(value); }

    ASTNode* get_value() { return value; }
    void set_value(ASTNode* value) { this->value = value; }

    virtual void print(const int padding) {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Index: " << std::endl;
        value->print(padding + 4);
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_index(this); }
    virtual NodeType get_tag() { return NT_INDEX; }
};

}
}

#endif /* _SLICE_NODE_ */
