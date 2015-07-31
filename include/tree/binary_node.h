#ifndef _BINARY_NODE_
#define _BINARY_NODE_

#include "tree/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class BinaryNode : public ASTNode {
private:
	ASTNode * lhs, * rhs;
	mtpython::parse::BinaryOper op;
public:
	BinaryNode(const int line_nr);
	~BinaryNode() { SAFE_DELETE(lhs); SAFE_DELETE(rhs); }

	ASTNode * get_lhs() { return this->lhs; }
	ASTNode * get_rhs() { return this->rhs; }
	void set_lhs(ASTNode * lhs) { this->lhs = lhs; }
	void set_rhs(ASTNode * rhs) { this->rhs = rhs; }
	mtpython::parse::BinaryOper get_op() { return this->op; }
	void set_op(const mtpython::parse::BinaryOper op) { this->op = op; } 

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Binary: "<< std::endl;
		std::cout << blank << "  " << line << ": Operator: "<< std::endl;
		std::cout << blank << "  " << line << ": Left: "<< std::endl;
		lhs->print(padding + 4);
		std::cout << blank << "  " << line << ": Right: "<< std::endl;
		rhs->print(padding + 4);
	}
	
	virtual NodeType get_tag() { return NT_BINARY; }
};

}
}

#endif /* _BINARY_NODE_ */
