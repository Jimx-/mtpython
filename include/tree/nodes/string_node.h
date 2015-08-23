#ifndef _STRING_NODE_
#define _STRING_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include "objects/base_object.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class StringNode : public ASTNode {
private:
	mtpython::objects::M_BaseObject* value;
public:
	StringNode(const int line_nr);
	~StringNode() {  }

	mtpython::objects::M_BaseObject* get_value() { return value; }
	void set_value(mtpython::objects::M_BaseObject* value) { this->value = value; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": String: ";
		value->dbg_print();
		std::cout << std::endl;
	}
	
	virtual void visit(ASTVisitor* visitor) { visitor->visit_string(this); }
	virtual NodeType get_tag() { return NT_STRING; }
};

}
}

#endif /* _STRING_NODE_ */
