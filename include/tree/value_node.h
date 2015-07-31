#ifndef _VALUE_NODE_
#define _VALUE_NODE_

#include "tree/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"
#include "type/types.h"

namespace mtpython { 
namespace tree {

class ValueNode : public Node {
private:
	mtpython::type::Value* val;
	mtpython::type::Type* type;
public:
	ValueNode(const int line_nr);
	~ValueNode() { SAFE_DELETE(val); SAFE_DELETE(type); }

	virtual void print(const int padding) {

	}
	
	virtual NodeType get_tag() { return NT_LITERAL; }
};

}
}

#endif /* _BINARY_NODE_ */
