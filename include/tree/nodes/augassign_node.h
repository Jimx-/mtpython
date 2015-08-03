#ifndef _AUGASSIGN_NODE_
#define _AUGASSIGN_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython { 
namespace tree {

class AugAssignNode : public ASTNode {
private:
	ASTNode* target;
	mtpython::parse::BinaryOper op;
	ASTNode* value;
public:
	AugAssignNode(const int line_nr);
	~AugAssignNode() { SAFE_DELETE(target); SAFE_DELETE(value); }

	ASTNode* get_value() { return value; }
	void set_value(ASTNode* value) { this->value = value; }
	ASTNode* get_target() { return target; }
	void set_target(ASTNode* target) { this->target = target; }
	mtpython::parse::BinaryOper get_op() { return this->op; }
	void set_op(const mtpython::parse::BinaryOper op) { this->op = op; } 
	
	virtual NodeType get_tag() { return NT_AUGASSIGN; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": AugAssign:" << std::endl;
		std::cout << blank << "  " << line << ": Operator: " << binop2str(op) << std::endl;
		std::cout << blank << "  " << line << ": Target:" << std::endl;
		target->print(padding + 4);
		std::cout << blank << "  " << line << ": Value:" << std::endl;
		value->print(padding + 4);
	}

	virtual void visit(ASTVisitor* visitor) { visitor->visit_augassign(this); }
};

}
}

#endif /* _AUGASSIGN_NODE_ */