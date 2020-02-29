#ifndef _GLOBAL_NODE_
#define _GLOBAL_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython {
namespace tree {

class GlobalNode : public ASTNode {
private:
    std::vector<std::string> names;

public:
    GlobalNode(const int line_nr);
    ~GlobalNode() {}

    std::vector<std::string>& get_names() { return names; }
    void push_name(const std::string& name) { names.push_back(name); }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Global" << std::endl;
    }
    virtual NodeType get_tag() { return NT_GLOBAL; }
    virtual void visit(ASTVisitor* visitor) { visitor->visit_global(this); }
};

} // namespace tree
} // namespace mtpython

#endif /* _GLOBAL_NODE_ */
