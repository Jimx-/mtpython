#ifndef _ATTRIBUTE_NODE_
#define _ATTRIBUTE_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <string>
#include "macros.h"

namespace mtpython {
namespace tree {

class AttributeNode : public ASTNode {
private:
    std::string attr;
    ASTNode* value;
    ExprContext ctx;

public:
    AttributeNode(const int line_nr);
    ~AttributeNode() { SAFE_DELETE(value); }

    std::string& get_attr() { return attr; }
    void set_attr(const std::string& attr) { this->attr = attr; }
    ASTNode* get_value() { return value; }
    void set_value(ASTNode* value) { this->value = value; }
    ExprContext get_context() { return ctx; }
    virtual void set_context(ExprContext ctx) { this->ctx = ctx; }

    virtual void print(const int padding) {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Attribute: " << std::endl;
        std::cout << blank << "  " << line << ": Value:" << std::endl;
        value->print(padding + 4);
        std::cout << blank << "  " << line << ": Attr: " << attr << ", Context: " << exprctx2str(ctx) << std::endl;
    }
    virtual NodeType get_tag() { return NT_ATTRIBUTE; }
    virtual void visit(ASTVisitor* visitor) { visitor->visit_attribute(this); }
};

}
}

#endif /* _ATTRIBUTE_NODE_ */
