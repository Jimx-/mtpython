#ifndef _BREAK_NODE_
#define _BREAK_NODE_

#include "tree/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class BreakNode : public ASTNode {
public:
	BreakNode(const int line_nr);
	~BreakNode() { }

	virtual NodeType get_tag() { return NT_BREAK; }
	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Break" << std::endl;
	}
};

}
}

#endif /* _BREAK_NODE_ */
