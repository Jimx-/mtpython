#ifndef _PASS_NODE_
#define _PASS_NODE_

#include "tree/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class PassNode : public ASTNode {
public:
	PassNode(const int line_nr);
	~PassNode() { }

	virtual NodeType get_tag() { return NT_PASS; }
	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Pass" << std::endl;
	}
};

}
}

#endif /* _PASS_NODE_ */
