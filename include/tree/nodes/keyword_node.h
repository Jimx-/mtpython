#ifndef _KEYWORD_NODE_
#define _KEYWORD_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <string>
#include "macros.h"

namespace mtpython {
namespace tree {

class KeywordNode : public ASTNode {
private:
    std::string arg;
    ASTNode* value;

public:
    KeywordNode(const int line_nr);
    ~KeywordNode() { SAFE_DELETE(value); }

    std::string& get_arg() { return arg; }
    void set_arg(const std::string& arg) { this->arg = arg; }
    ASTNode* get_value() { return value; }
    void set_value(ASTNode* value) { this->value = value; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Keyword: " << std::endl;
        std::cout << blank << "  " << line << ": Arg: " << arg << std::endl;
        std::cout << blank << "  " << line << ": Value:" << std::endl;
        value->print(padding + 4);
    }
    virtual NodeType get_tag() { return NT_KEYWORD; }
    virtual void visit(ASTVisitor* visitor) { visitor->visit_keyword(this); }
};

} // namespace tree
} // namespace mtpython

#endif /* _KEYWORD_NODE_ */
