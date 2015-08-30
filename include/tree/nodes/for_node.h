#ifndef _FOR_NODE_
#define _FOR_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include "macros.h"

namespace mtpython { 
namespace tree {

class ForNode : public ASTNode {
private:
	ASTNode* target, *iter, *body, *orelse; 
public:
	ForNode(const int line_nr);
	~ForNode()
	{
		SAFE_DELETE(target);
		SAFE_DELETE(iter);
		ASTNode* node = body, *prev;
		while (node) {
			prev = node;
			node = node->get_sibling();
			SAFE_DELETE(prev);
		}
		node = orelse;
		while (node) {
			prev = node;
			node = node->get_sibling();
			SAFE_DELETE(prev);
		}
	}

	ASTNode * get_target() { return target; }
	void set_target(ASTNode * target) { this->target = target; }
	ASTNode * get_iter() { return iter; }
	void set_iter(ASTNode * iter) { this->iter = iter; }
	ASTNode * get_body() { return body; }
	void set_body(ASTNode * body) { this->body = body; }
	ASTNode * get_orelse() { return orelse; }
	void set_orelse(ASTNode * orelse) { this->orelse = orelse; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": For:" << std::endl;
		std::cout << blank << "  " << line << ": Target:" << std::endl;
		target->print(padding + 4);

		std::cout << blank << "  " << line << ": Iter:" << std::endl;
		iter->print(padding + 4);
		
		std::cout << blank << "  " << line << ": Body:" << std::endl;
		ASTNode* stmt = body;
		while (stmt) {
			stmt->print(padding + 4);
			stmt = stmt->get_sibling();
		}
		
		if (orelse) {
			std::cout << blank << line << ": Else:" << std::endl;
			stmt = orelse;
			while (stmt) {
				stmt->print(padding + 4);
				stmt = stmt->get_sibling();
			}
		}
	}
	virtual NodeType get_tag() { return NT_FOR; }

	virtual void visit(ASTVisitor* visitor) { visitor->visit_for(this); }
};

}
}

#endif /* _FOR_NODE_ */
