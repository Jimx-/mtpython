#ifndef _RAISE_NODE_
#define _RAISE_NODE_

#include "tree/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class RaiseNode : public ASTNode {
private:
	ASTNode* exc;
	ASTNode* cause;
public:
	RaiseNode(const int line_nr);
	~RaiseNode() { SAFE_DELETE(exc); SAFE_DELETE(cause); }

	ASTNode* get_exc() { return exc; }
	void set_exc(ASTNode* exc) { this->exc = exc; }
	ASTNode* get_cause() { return cause; }
	void set_cause(ASTNode* cause) { this->cause = cause; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Raise: "<< std::endl;
		std::cout << blank << "  " << line << ": Exception: "<< std::endl;
		if (exc) exc->print(padding + 4);
		std::cout << blank << "  " << line << ": Cause: "<< std::endl;
		if (cause) cause->print(padding + 4);
	}
	
	virtual NodeType get_tag() { return NT_RAISE; }

	virtual void visit(ASTVisitor* visitor) { visitor->visit_raise(this); }
};

}
}

#endif /* _RAISE_NODE_ */
