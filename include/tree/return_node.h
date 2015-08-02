#ifndef _RETURN_NODE_
#define _RETURN_NODE_

#include "tree/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class ReturnNode : public ASTNode {
private:
	ASTNode* value;
public:
	ReturnNode(const int line_nr);
	~ReturnNode() { SAFE_DELETE(value); }

	ASTNode* get_value() { return value; }
	void set_value(ASTNode* value) { this->value = value; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Return: "<< std::endl;
		std::cout << blank << "  " << line << ": Value: "<< std::endl;
		if (value) value->print(padding + 4);
	}
	
	virtual NodeType get_tag() { return NT_RETURN; }
};

}
}

#endif /* _RETURN_NODE_ */
