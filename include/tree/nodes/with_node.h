#ifndef _WITH_NODE_
#define _WITH_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython {
namespace tree {

class WithItemNode : public ASTNode {
private:
    ASTNode *context_expr, *optional_vars;

public:
    WithItemNode(const int line_nr);
    ~WithItemNode()
    {
        SAFE_DELETE(context_expr);
        SAFE_DELETE(optional_vars);
    }

    ASTNode* get_context_expr() { return context_expr; }
    void set_context_expr(ASTNode* context_expr)
    {
        this->context_expr = context_expr;
    }
    ASTNode* get_optional_vars() { return optional_vars; }
    void set_optional_vars(ASTNode* optional_vars)
    {
        this->optional_vars = optional_vars;
    }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": WithItem:" << std::endl;
        std::cout << blank << "  " << line << ": Context Expr:" << std::endl;
        context_expr->print(padding + 4);
        if (optional_vars) {
            std::cout << blank << "  " << line
                      << ": Optional Vars:" << std::endl;
            optional_vars->print(padding + 4);
        }
    }

    virtual NodeType get_tag() { return NT_WITHITEM; }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_withitem(this); }
};

class WithNode : public ASTNode {
private:
    std::vector<WithItemNode*> items;
    ASTNode* body;

public:
    WithNode(const int line_nr);
    ~WithNode()
    {
        ASTNode *node = body, *prev;
        while (node) {
            prev = node;
            node = node->get_sibling();
            SAFE_DELETE(prev);
        }

        for (std::size_t i = 0; i < items.size(); i++)
            SAFE_DELETE(items[i]);
    }

    std::vector<WithItemNode*>& get_items() { return items; }
    void push_item(WithItemNode* item) { items.push_back(item); }
    ASTNode* get_body() { return body; }
    void set_body(ASTNode* body) { this->body = body; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": With:" << std::endl;
        std::cout << blank << "  " << line << ": Items:" << std::endl;
        for (auto item : items)
            item->print(padding + 4);

        std::cout << blank << "  " << line << ": Body:" << std::endl;
        ASTNode* stmt = body;
        while (stmt) {
            stmt->print(padding + 4);
            stmt = stmt->get_sibling();
        }
    }

    virtual NodeType get_tag() { return NT_WITH; }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_with(this); }
};

} // namespace tree
} // namespace mtpython

#endif /* _WITH_NODE_ */
