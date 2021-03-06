#ifndef _IMPORTFROM_NODE_
#define _IMPORTFROM_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython {
namespace tree {

class ImportFromNode : public ASTNode {
private:
    std::string module;
    std::vector<AliasNode*> names;
    int level;

public:
    ImportFromNode(const int line_nr);
    ~ImportFromNode()
    {
        for (std::size_t i = 0; i < names.size(); i++)
            SAFE_DELETE(names[i]);
    }

    std::string get_module() { return module; }
    void set_module(const std::string& module) { this->module = module; }
    std::vector<AliasNode*>& get_names() { return names; }
    void push_name(AliasNode* name) { names.push_back(name); }
    int get_level() { return level; }
    void set_level(int level) { this->level = level; }

    virtual NodeType get_tag() { return NT_IMPORTFROM; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": ImportFrom:" << std::endl;
        std::cout << blank << "  " << line << ": Module: " << module
                  << std::endl;
        std::cout << blank << "  " << line << ": Names:" << std::endl;
        for (unsigned int i = 0; i < names.size(); i++)
            names[i]->print(padding + 4);
        std::cout << blank << "  " << line << ": Level: " << level << std::endl;
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_importfrom(this); }
};

} // namespace tree
} // namespace mtpython

#endif /* _IMPORTFROM_NODE_ */
