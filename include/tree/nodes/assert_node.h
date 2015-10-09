#ifndef _ASSERT_NODE_
#define _ASSERT_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class ASTVisitor;

class AssertNode : public ASTNode {
private:
	ASTNode* test;
	ASTNode* msg;
public:
	AssertNode(const int line_nr);
	~AssertNode()
	{
		SAFE_DELETE(test);
		SAFE_DELETE(msg);
	}

	ASTNode* get_test() { return this->test; }
	void set_test(ASTNode * test) { this->test = test; }
	ASTNode* get_msg() { return this->msg; }
	void set_msg(ASTNode * msg) { this->msg = msg; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Assert: "<< std::endl;
		std::cout << blank << "  " << line << ": Test: "<< std::endl;
		test->print(padding + 4);
		if (msg) {
			std::cout << blank << "  " << line << ": Msg: "<< std::endl;
			msg->print(padding + 4);
		}
	}
	
	virtual void visit(ASTVisitor* visitor) { visitor->visit_assert(this); }

	virtual NodeType get_tag() { return NT_ASSERT; }
};

}
}

#endif /* _ASSERT_NODE_ */
