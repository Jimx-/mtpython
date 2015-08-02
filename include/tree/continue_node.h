#ifndef _CONTINUE_NODE_
#define _CONTINUE_NODE_

#include "tree/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class ContinueNode : public ASTNode {
public:
	ContinueNode(const int line_nr);
	~ContinueNode() { }

	virtual NodeType get_tag() { return NT_CONTINUE; }
	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Continue" << std::endl;
	}
};

}
}

#endif /* _CONTINUE_NODE_ */
