#ifndef _TUPLE_NODE_
#define _TUPLE_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython { 
namespace tree {

class TupleNode : public ASTNode {
private:
	std::vector<ASTNode*> elts;
	ExprContext ctx;
public:
	TupleNode(const int line_nr);
	~TupleNode() { elts.clear(); }

	void push_element(ASTNode* elt) { elts.push_back(elt); }
	std::vector<ASTNode*>& get_elements() { return elts; }
	virtual void set_context(ExprContext ctx) { 
		this->ctx = ctx; 
		for (unsigned int i = 0; i < elts.size(); i++) elts[i]->set_context(ctx);
	}

	virtual NodeType get_tag() { return NT_TUPLE; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Tuple:" << std::endl;
		std::cout << blank << "  " << line << ": Elements:" << std::endl;
		for (unsigned int i = 0; i < elts.size(); i++) elts[i]->print(padding + 4);
	}

	virtual void visit(ASTVisitor* visitor) { visitor->visit_tuple(this); }
};

}
}

#endif /* _TUPLE_NODE_ */
