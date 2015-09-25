#ifndef _FUNCTIONDEF_NODE_
#define _FUNCTIONDEF_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class FunctionDefNode : public ASTNode {
private:
	std::string name;
	ASTNode* args;
	ASTNode* body;
public:
	FunctionDefNode(const int line_nr);
	~FunctionDefNode()
	{
		SAFE_DELETE(args);
		ASTNode* node = body, *prev;
		while (node) {
			prev = node;
			node = node->get_sibling();
			SAFE_DELETE(prev);
		}
	}

	std::string get_name() { return this->name; }
	void set_name(const std::string& name) { this->name = name; }
	ASTNode* get_args() { return this->args; }
	void set_args(ASTNode* args) { this->args = args; }
	ASTNode* get_body() { return this->body; }
	void set_body(ASTNode * body) { this->body = body; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": FunctionDef: "<< std::endl;
		std::cout << blank << "  " << line << ": Args: "<< std::endl;
		args->print(padding + 4);
		std::cout << blank << "  " << line << ": Body: "<< std::endl;
		ASTNode* stmt = body;
		while (stmt) {
			stmt->print(padding + 4);
			stmt = stmt->get_sibling();
		}
	}
	
	virtual NodeType get_tag() { return NT_FUNCDEF; }

	virtual void visit(ASTVisitor* visitor) { visitor->visit_functiondef(this); }
};

}
}

#endif /* _FUNCTIONDEF_NODE_ */
