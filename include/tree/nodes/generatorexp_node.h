#ifndef _GENERATOREXP_H_
#define _GENERATOREXP_H_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython {
namespace tree {

class GeneratorExpNode : public ASTNode {
private:
    ASTNode* elt;
    std::vector<ComprehensionNode*> comprehensions;
public:
    GeneratorExpNode(const int line_nr);
    ~GeneratorExpNode()
    {
        for (std::size_t i = 0; i < comprehensions.size(); i++)
            SAFE_DELETE(comprehensions[i]);
        SAFE_DELETE(elt);
    }

    std::vector<ComprehensionNode*>& get_comprehensions() { return comprehensions; }
    void push_comprehension(ComprehensionNode* comprehension) { comprehensions.push_back(comprehension); }
    ASTNode * get_elt() { return elt; }
    void set_elt(ASTNode * elt) { this->elt = elt; }

    virtual NodeType get_tag() { return NT_GENERATOREXP; }

    virtual void print(const int padding) {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": GeneratorExp:" << std::endl;

        std::cout << blank << "  " << line << ": Elt:" << std::endl;
        elt->print(padding + 4);

        if (comprehensions.size() > 0)
            std::cout << blank << "  " << line << ": Comprehensions:" << std::endl;
        for (unsigned int i = 0; i < comprehensions.size(); i++) comprehensions[i]->print(padding + 4);
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_generatorexp(this); }
};

}
}

#endif /* _GENERATOREXP_H_ */