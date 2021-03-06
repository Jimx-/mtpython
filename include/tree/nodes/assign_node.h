#ifndef _ASSIGN_NODE_
#define _ASSIGN_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython {
namespace tree {

class AssignNode : public ASTNode {
private:
    std::vector<ASTNode*> targets;
    ASTNode* value;

public:
    AssignNode(const int line_nr);
    ~AssignNode()
    {
        SAFE_DELETE(value);
        for (std::size_t i = 0; i < targets.size(); i++)
            SAFE_DELETE(targets[i]);
    }

    ASTNode* get_value() { return value; }
    void set_value(ASTNode* value) { this->value = value; }
    std::vector<ASTNode*>& get_targets() { return targets; }
    void push_target(ASTNode* target) { targets.push_back(target); }
    void set_targets(std::vector<ASTNode*>& targets)
    {
        this->targets.insert(this->targets.end(), targets.begin(),
                             targets.end());
    }

    virtual NodeType get_tag() { return NT_ASSIGN; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Assign:" << std::endl;
        std::cout << blank << "  " << line << ": Targets:" << std::endl;
        for (unsigned int i = 0; i < targets.size(); i++)
            targets[i]->print(padding + 4);
        std::cout << blank << "  " << line << ": Value:" << std::endl;
        value->print(padding + 4);
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_assign(this); }
};

} // namespace tree
} // namespace mtpython

#endif /* _ASSIGN_NODE_ */
