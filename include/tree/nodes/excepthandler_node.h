#ifndef _EXCEPTHANDLER_NODE_
#define _EXCEPTHANDLER_NODE_

#include <string>

namespace mtpython {
namespace tree {

class ExceptHandlerNode : public ASTNode {
private:
    std::string name;
    ASTNode* type;
    ASTNode* body;

public:
    ExceptHandlerNode(const int line_nr);
    ~ExceptHandlerNode()
    {
        SAFE_DELETE(type);
        ASTNode *node = body, *prev;
        while (node) {
            prev = node;
            node = node->get_sibling();
            SAFE_DELETE(prev);
        }
    }

    std::string& get_name() { return name; }
    void set_name(const std::string& name) { this->name = name; }
    ASTNode* get_type() { return type; }
    void set_type(ASTNode* type) { this->type = type; }
    ASTNode* get_body() { return body; }
    void set_body(ASTNode* body) { this->body = body; }

    virtual void print(const int padding)
    {
        std::string blank(padding, ' ');
        std::cout << blank << line << ": ExceptHandler: " << std::endl;
        if (type) {
            std::cout << blank << "  " << line << ": Type:" << std::endl;
            type->print(padding + 4);
        }
        std::cout << blank << "  " << line << ": Name: " << name << std::endl;
        std::cout << blank << "  " << line << ": Body:" << std::endl;
        ASTNode* stmt = body;
        while (stmt) {
            stmt->print(padding + 4);
            stmt = stmt->get_sibling();
        }
    }
    virtual NodeType get_tag() { return NT_EXCEPTHANDLER; }
    virtual void visit(ASTVisitor* visitor)
    {
        visitor->visit_excepthandler(this);
    }
};

} // namespace tree
} // namespace mtpython

#endif // _EXCEPTHANDLER_NODE_
