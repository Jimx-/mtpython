#ifndef _VALUE_NODE_
#define _VALUE_NODE_

#include "tree/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class NumberNode : public ASTNode {
private:
public:
	NumberNode(const int line_nr);
	~NumberNode() {  }

	virtual void print(const int padding) {

	}
	
	virtual NodeType get_tag() { return NT_NUMBER; }
};

}
}

#endif /* _BINARY_NODE_ */
