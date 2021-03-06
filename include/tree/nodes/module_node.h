#ifndef _MODULE_NODE_
#define _MODULE_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython {
namespace tree {

class ASTVisitor;

class ModuleNode : public ASTNode {
private:
    ASTNode* body;

public:
    ModuleNode(const int line_nr);
    ~ModuleNode()
    {
        ASTNode *node = body, *prev;
        while (node) {
            prev = node;
            node = node->get_sibling();
            SAFE_DELETE(prev);
        }
    }

    ASTNode* get_body() { return this->body; }
    void set_body(ASTNode* body) { this->body = body; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Module: " << std::endl;
        std::cout << blank << "  " << line << ": Body: " << std::endl;
        ASTNode* stmt = body;
        while (stmt) {
            stmt->print(padding + 4);
            stmt = stmt->get_sibling();
        }
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_module(this); }

    virtual NodeType get_tag() { return NT_MODULE; }
};

} // namespace tree
} // namespace mtpython

#endif /* _MODULE_NODE_ */
