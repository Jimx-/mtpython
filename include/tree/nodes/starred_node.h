#ifndef _STARRED_NODE_
#define _STARRED_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class StarredNode : public ASTNode {
private:
	ASTNode* value;
	ExprContext ctx;

public:
	StarredNode(const int line_nr);
	~StarredNode() { SAFE_DELETE(value); }

	ASTNode* get_value() { return value; }
	void set_value(ASTNode* value) { this->value = value; }
	ExprContext get_context() { return ctx; }
	virtual void set_context(ExprContext ctx) { this->ctx = ctx; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Starred:  Context: " << exprctx2str(ctx) << std::endl;
		std::cout << blank << "  " << line << ": Value:" << std::endl;
		value->print(padding + 4);
	}
	virtual NodeType get_tag() { return NT_STARRED; }
	virtual void visit(ASTVisitor* visitor) { visitor->visit_starred(this); }
};

}
}

#endif /* _STARRED_NODE_ */
