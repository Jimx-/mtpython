#ifndef _COMPREHENSION_H_
#define _COMPREHENSION_H_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython {
namespace tree {

class ComprehensionNode : public ASTNode {
private:
    ASTNode* target;
    ASTNode* iter;
    std::vector<ASTNode*> ifs;
public:
    ComprehensionNode(const int line_nr);
    ~ComprehensionNode()
    {
        for (std::size_t i = 0; i < ifs.size(); i++)
            SAFE_DELETE(ifs[i]);
        SAFE_DELETE(target);
        SAFE_DELETE(iter);
    }

    std::vector<ASTNode*>& get_ifs() { return ifs; }
    void push_if(ASTNode* _if) { ifs.push_back(_if); }
    ASTNode * get_target() { return target; }
    void set_target(ASTNode * target) { this->target = target; }
    ASTNode * get_iter() { return iter; }
    void set_iter(ASTNode * iter) { this->iter = iter; }

    virtual NodeType get_tag() { return NT_COMPREHENSION; }

    virtual void print(const int padding) {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Comprehension:" << std::endl;

        std::cout << blank << "  " << line << ": Target:" << std::endl;
        target->print(padding + 4);

        std::cout << blank << "  " << line << ": Iter:" << std::endl;
        iter->print(padding + 4);

        if (ifs.size() > 0)
            std::cout << blank << "  " << line << ": Ifs:" << std::endl;
        for (unsigned int i = 0; i < ifs.size(); i++) ifs[i]->print(padding + 4);
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_comprehension(this); }
};

}
}

#endif /* _COMPREHENSION_H_ */