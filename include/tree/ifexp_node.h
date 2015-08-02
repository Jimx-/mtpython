#ifndef _IFEXP_NODE_
#define _IFEXP_NODE_

#include "tree/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class IfExpNode : public ASTNode {
private:
	ASTNode* test, *body, *orelse;
public:
	IfExpNode(const int line_nr);
	~IfExpNode() { SAFE_DELETE(test); SAFE_DELETE(body); SAFE_DELETE(orelse); }

	ASTNode* get_test() { return test; }
	void set_test(ASTNode* test) { this->test = test; }
	ASTNode* get_body() { return body; }
	void set_body(ASTNode* body) { this->body = body; }
	ASTNode* get_orelse() { return orelse; }
	void set_orelse(ASTNode* orelse) { this->orelse = orelse; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": IfExp:" << std::endl;
		std::cout << blank << "  " << line << ": Test:" << std::endl;
		test->print(padding + 4);
		std::cout << blank << "  " << line << ": Body:" << std::endl;
		body->print(padding + 4);
		std::cout << blank << "  " << line << ": Else:" << std::endl;
		orelse->print(padding + 4);
	}

	virtual NodeType get_tag() { return NT_IFEXP; }
};
}
}

#endif /* _IFEXP_NODE_ */
