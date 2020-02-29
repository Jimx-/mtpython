#ifndef _CLASSDEF_NODE_
#define _CLASSDEF_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython {
namespace tree {

class ClassDefNode : public ASTNode {
private:
    std::string name;
    std::vector<ASTNode*> bases;
    std::vector<KeywordNode*> keywords;
    ASTNode* body;

public:
    ClassDefNode(const int line_nr);
    ~ClassDefNode()
    {
        ASTNode *node = body, *prev;
        while (node) {
            prev = node;
            node = node->get_sibling();
            SAFE_DELETE(prev);
        }

        for (std::size_t i = 0; i < bases.size(); i++)
            SAFE_DELETE(bases[i]);
        for (std::size_t i = 0; i < keywords.size(); i++)
            SAFE_DELETE(keywords[i]);
    }

    std::string get_name() { return this->name; }
    void set_name(const std::string& name) { this->name = name; }
    ASTNode* get_body() { return this->body; }
    void set_body(ASTNode* body) { this->body = body; }
    std::vector<ASTNode*>& get_bases() { return bases; }
    void set_bases(const std::vector<ASTNode*>& bases) { this->bases = bases; }
    std::vector<KeywordNode*>& get_keywords() { return keywords; }
    void set_keywords(const std::vector<KeywordNode*>& keywords)
    {
        this->keywords = keywords;
    }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": ClassDef: " << std::endl;
        std::cout << blank << "  " << line << ": Bases: " << std::endl;
        for (auto& base : bases)
            base->print(padding + 4);
        std::cout << blank << "  " << line << ": Keywords: " << std::endl;
        for (auto& keyword : keywords)
            keyword->print(padding + 4);
        std::cout << blank << "  " << line << ": Body: " << std::endl;
        ASTNode* stmt = body;
        while (stmt) {
            stmt->print(padding + 4);
            stmt = stmt->get_sibling();
        }
    }

    virtual NodeType get_tag() { return NT_CLASSDEF; }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_classdef(this); }
};

} // namespace tree
} // namespace mtpython

#endif /* _CLASSDEF_NODE_ */
