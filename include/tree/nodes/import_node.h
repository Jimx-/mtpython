#ifndef _IMPORT_NODE_
#define _IMPORT_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython {
namespace tree {

class ImportNode : public ASTNode {
private:
    std::vector<AliasNode*> names;

public:
    ImportNode(const int line_nr);
    ~ImportNode()
    {
        for (std::size_t i = 0; i < names.size(); i++)
            SAFE_DELETE(names[i]);
    }

    std::vector<AliasNode*>& get_names() { return names; }
    void push_name(AliasNode* name) { names.push_back(name); }

    virtual NodeType get_tag() { return NT_IMPORT; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Import:" << std::endl;
        std::cout << blank << "  " << line << ": Names:" << std::endl;
        for (unsigned int i = 0; i < names.size(); i++)
            names[i]->print(padding + 4);
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_import(this); }
};

} // namespace tree
} // namespace mtpython

#endif /* _IMPORT_NODE_ */
