#ifndef _SLICE_NODE_
#define _SLICE_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include "objects/base_object.h"
#include <iostream>
#include "macros.h"

namespace mtpython {
namespace tree {

class IndexNode : public ASTNode {
private:
    ASTNode* value;

public:
    IndexNode(const int line_nr);
    ~IndexNode() { SAFE_DELETE(value); }

    ASTNode* get_value() { return value; }
    void set_value(ASTNode* value) { this->value = value; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Index: " << std::endl;
        value->print(padding + 4);
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_index(this); }
    virtual NodeType get_tag() { return NT_INDEX; }
};

class SliceNode : public ASTNode {
private:
    ASTNode *lower, *upper, *step;

public:
    SliceNode(const int line_nr);
    ~SliceNode()
    {
        SAFE_DELETE(lower);
        SAFE_DELETE(upper);
        SAFE_DELETE(step);
    }

    ASTNode* get_lower() { return lower; }
    void set_lower(ASTNode* lower) { this->lower = lower; }
    ASTNode* get_upper() { return upper; }
    void set_upper(ASTNode* upper) { this->upper = upper; }
    ASTNode* get_step() { return step; }
    void set_step(ASTNode* step) { this->step = step; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": Slice: " << std::endl;
        if (lower) {
            std::cout << blank << "  " << line << ": Lower: " << std::endl;
            lower->print(padding + 4);
        }
        if (upper) {
            std::cout << blank << "  " << line << ": Upper: " << std::endl;
            upper->print(padding + 4);
        }
        if (step) {
            std::cout << blank << "  " << line << ": Step: " << std::endl;
            step->print(padding + 4);
        }
    }

    virtual void visit(ASTVisitor* visitor) { visitor->visit_slice(this); }
    virtual NodeType get_tag() { return NT_SLICE; }
};

} // namespace tree
} // namespace mtpython

#endif /* _SLICE_NODE_ */
