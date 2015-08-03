#ifndef _IF_NODE_
#define _IF_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class IfNode : public ASTNode {
private:
	ASTNode* test, *body, *orelse; 
public:
	IfNode(const int line_nr);
	~IfNode() { SAFE_DELETE(test); SAFE_DELETE(body); SAFE_DELETE(orelse); }

	ASTNode * get_test() { return test; }
	void set_test(ASTNode * test) { this->test = test; }
	ASTNode * get_body() { return body; }
	void set_body(ASTNode * body) { this->body = body; }
	ASTNode * get_orelse() { return orelse; }
	void set_orelse(ASTNode * orelse) { this->orelse = orelse; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": If:" << std::endl;
		std::cout << blank << "  " << line << ": Test:" << std::endl;
		test->print(padding + 4);
		
		std::cout << blank << "  " << line << ": Body:" << std::endl;
		ASTNode* stmt = body;
		while (stmt) {
			stmt->print(padding + 4);
			stmt = stmt->get_sibling();
		}
		
		if (orelse) {
			std::cout << blank << "  " << line << ": Else:" << std::endl;
			stmt = orelse;
			while (stmt) {
				stmt->print(padding + 4);
				stmt = stmt->get_sibling();
			}
		}
	}

	virtual NodeType get_tag() { return NT_IF; }

	virtual void visit(ASTVisitor* visitor) { visitor->visit_if(this); }
};

}
}

#endif /* _IF_NODE_ */
