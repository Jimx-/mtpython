#ifndef _KEYWORD_NODE_
#define _KEYWORD_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class KeywordNode : public ASTNode {
private:
	ASTNode* arg;
	ASTNode* value;

public:
	KeywordNode(const int line_nr);
	~KeywordNode() {}

	ASTNode* get_arg() { return arg; }
	void set_arg(ASTNode* arg) { this->arg = arg; }
	ASTNode* get_value() { return value; }
	void set_value(ASTNode* value) { this->value = value; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Keyword: " << std::endl;
		std::cout << blank << "  " << line << ": Arg:" << std::endl;
		arg->print(padding + 4);
		std::cout << blank << "  " << line << ": Value:" << std::endl;
		value->print(padding + 4);
	}
	virtual NodeType get_tag() { return NT_KEYWORD; }
	virtual void visit(ASTVisitor* visitor) { visitor->visit_keyword(this); }
};

}
}

#endif /* _KEYWORD_NODE_ */
