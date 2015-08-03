#ifndef _IDENT_NODE_
#define _IDENT_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class NameNode : public ASTNode {
private:
	std::string name;
	ExprContext ctx;

public:
	NameNode(const int line_nr);
	~NameNode() {}

	std::string get_name() { return name; }
	void set_name(const std::string& name) { this->name = name; }
	ExprContext get_context() { return ctx; }
	virtual void set_context(ExprContext ctx) { this->ctx = ctx; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Ident: " << name << ", Context: " << exprctx2str(ctx) << std::endl;
	}
	virtual NodeType get_tag() { return NT_IDENT; }
	virtual void visit(ASTVisitor* visitor) { visitor->visit_name(this); }
};

}
}

#endif /* _FUNCTIONDEF_NODE_ */
