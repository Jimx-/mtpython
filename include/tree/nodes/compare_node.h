#ifndef _COMPARE_NODE_
#define _COMPARE_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython { 
namespace tree {

class CompareNode : public ASTNode {
private:
	ASTNode* left;
	std::vector<mtpython::parse::CmpOper> ops;
	std::vector<ASTNode*> comparators;
public:
	CompareNode(const int line_nr);
	~CompareNode() { SAFE_DELETE(left); }

	ASTNode* get_left() { return this->left; }
	void set_left(ASTNode* left) { this->left = left; }
	void push_op(mtpython::parse::CmpOper op) { ops.push_back(op); }
	void push_comparator(ASTNode* comparator) { comparators.push_back(comparator); }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Compare: "<< std::endl;
		std::cout << blank << "  " << line << ": Left: "<< std::endl;
		left->print(padding + 4);
		std::cout << blank << "  " << line << ": Operators: ";
		for (unsigned int i = 0; i < ops.size(); i++) std::cout << cmpop2str(ops[i]) << " ";
		std::cout << std::endl;
		std::cout << blank << "  " << line << ": Comparators: " << std::endl;
		for (unsigned int i = 0; i < comparators.size(); i++) comparators[i]->print(padding + 4);
	}
	
	virtual NodeType get_tag() { return NT_COMPARE; }

	virtual void visit(ASTVisitor* visitor) { visitor->visit_compare(this); }
};

}
}

#endif /* _COMPARE_NODE_ */
