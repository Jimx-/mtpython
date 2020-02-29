#ifndef _SET_NODE_
#define _SET_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython {
namespace tree {

class SetNode : public ASTNode {
private:
    std::vector<ASTNode*> elts;

public:
    SetNode(const int line_nr);
    ~SetNode()
    {
        for (std::size_t i = 0; i < elts.size(); i++)
            SAFE_DELETE(elts[i]);
    }

    void push_element(ASTNode* elt) { elts.push_back(elt); }
    std::vector<ASTNode*>& get_elements() { return elts; }

    virtual NodeType get_tag() { return NT_SET; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Set:" << std::endl;
        std::cout << blank << "  " << line << ": Elements:" << std::endl;
        for (unsigned int i = 0; i < elts.size(); i++)
            elts[i]->print(padding + 4);
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_set(this); }
};

} // namespace tree
} // namespace mtpython

#endif /* _SET_NODE_ */
