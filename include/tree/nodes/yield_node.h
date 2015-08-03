#ifndef _YIELD_NODE_
#define _YIELD_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class YieldNode : public ASTNode {
private:
	ASTNode* value;
public:
	YieldNode(const int line_nr);
	~YieldNode() { SAFE_DELETE(value); }

	ASTNode* get_value() { return value; }
	void set_value(ASTNode* value) { this->value = value; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Yield: "<< std::endl;
		std::cout << blank << "  " << line << ": Value: "<< std::endl;
		if (value) value->print(padding + 4);
	}
	
	virtual NodeType get_tag() { return NT_YIELD; }

	virtual void visit(ASTVisitor* visitor) { visitor->visit_yield(this); }
};

}
}

#endif /* _YIELD_NODE_ */
