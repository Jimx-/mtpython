#ifndef _UNARYOP_NODE_
#define _UNARYOP_NODE_

#include "tree/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class UnaryOpNode : public ASTNode {
private:
	ASTNode* operand;
	mtpython::parse::UnaryOper op;
public:
	UnaryOpNode(const int line_nr);
	~UnaryOpNode() { SAFE_DELETE(operand); }

	ASTNode* get_operand() { return this->operand; }
	void set_operand(ASTNode* operand) { this->operand = operand; }
	mtpython::parse::UnaryOper get_op() { return this->op; }
	void set_op(const mtpython::parse::UnaryOper op) { this->op = op; } 
	virtual NodeType get_tag() { return NT_UNARY; }

	virtual void visit(ASTVisitor* visitor) { visitor->visit_unaryop(this); }
};

}
}

#endif	/* _UNARYOP_NODE_ */
