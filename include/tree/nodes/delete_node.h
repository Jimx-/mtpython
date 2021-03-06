#ifndef _DELETE_NODE_
#define _DELETE_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython {
namespace tree {

class DeleteNode : public ASTNode {
private:
    std::vector<ASTNode*> targets;

public:
    DeleteNode(const int line_nr);
    ~DeleteNode()
    {
        for (std::size_t i = 0; i < targets.size(); i++)
            SAFE_DELETE(targets[i]);
    }

    std::vector<ASTNode*>& get_targets() { return targets; }
    void push_target(ASTNode* target) { targets.push_back(target); }
    virtual NodeType get_tag() { return NT_DELETE; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Delete:" << std::endl;
        for (unsigned int i = 0; i < targets.size(); i++)
            targets[i]->print(padding + 4);
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_delete(this); }
};

} // namespace tree
} // namespace mtpython

#endif /* _DELETE_NODE_ */
