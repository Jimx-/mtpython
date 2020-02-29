#ifndef _TRY_NODE_
#define _TRY_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython {
namespace tree {

class TryNode : public ASTNode {
private:
    ASTNode* body;
    std::vector<ExceptHandlerNode*> handlers;
    ASTNode* orelse;
    ASTNode* finalbody;

public:
    TryNode(const int line_nr);
    ~TryNode()
    {
        for (std::size_t i = 0; i < handlers.size(); i++)
            SAFE_DELETE(handlers[i]);
        ASTNode *node = body, *prev;
        while (node) {
            prev = node;
            node = node->get_sibling();
            SAFE_DELETE(prev);
        }
        node = orelse;
        while (node) {
            prev = node;
            node = node->get_sibling();
            SAFE_DELETE(prev);
        }
        node = finalbody;
        while (node) {
            prev = node;
            node = node->get_sibling();
            SAFE_DELETE(prev);
        }
    }

    std::vector<ExceptHandlerNode*>& get_handlers() { return handlers; }
    void push_handler(ExceptHandlerNode* handler)
    {
        handlers.push_back(handler);
    }
    ASTNode* get_body() { return body; }
    void set_body(ASTNode* body) { this->body = body; }
    ASTNode* get_orelse() { return orelse; }
    void set_orelse(ASTNode* orelse) { this->orelse = orelse; }
    ASTNode* get_finalbody() { return finalbody; }
    void set_finalbody(ASTNode* finalbody) { this->finalbody = finalbody; }

    virtual NodeType get_tag() { return NT_TRY; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Try:" << std::endl;

        std::cout << blank << "  " << line << ": Body:" << std::endl;
        ASTNode* stmt = body;
        while (stmt) {
            stmt->print(padding + 4);
            stmt = stmt->get_sibling();
        }

        if (handlers.size() > 0)
            std::cout << blank << "  " << line << ": Handlers:" << std::endl;
        for (unsigned int i = 0; i < handlers.size(); i++)
            handlers[i]->print(padding + 4);
        if (orelse) {
            std::cout << blank << "  " << line << ": Else:" << std::endl;
            stmt = orelse;
            while (stmt) {
                stmt->print(padding + 4);
                stmt = stmt->get_sibling();
            }
        }

        if (finalbody) {
            std::cout << blank << "  " << line << ": Finalbody:" << std::endl;
            stmt = finalbody;
            while (stmt) {
                stmt->print(padding + 4);
                stmt = stmt->get_sibling();
            }
        }
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_try(this); }
};

} // namespace tree
} // namespace mtpython

#endif /* _TRY_NODE_ */