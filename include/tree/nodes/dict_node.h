#ifndef _DICT_NODE_
#define _DICT_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython { 
namespace tree {

class DictNode : public ASTNode {
private:
	std::vector<ASTNode*> keys;
	std::vector<ASTNode*> values;
public:
	DictNode(const int line_nr);
	~DictNode()
	{
		for (std::size_t i = 0; i < keys.size(); i++) SAFE_DELETE(keys[i]);
		for (std::size_t i = 0; i < values.size(); i++) SAFE_DELETE(values[i]);
	}

	void push_key(ASTNode* elt) { keys.push_back(elt); }
	std::vector<ASTNode*>& get_keys() { return keys; }
	void push_value(ASTNode* elt) { values.push_back(elt); }
	std::vector<ASTNode*>& get_values() { return values; }

	virtual NodeType get_tag() { return NT_DICT; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Dict:" << std::endl;
		std::cout << blank << "  " << line << ": Keys:" << std::endl;
		for (unsigned int i = 0; i < keys.size(); i++) keys[i]->print(padding + 4);
		std::cout << blank << "  " << line << ": Values:" << std::endl;
		for (unsigned int i = 0; i < values.size(); i++) values[i]->print(padding + 4);
	}

	virtual void visit(ASTVisitor* visitor) { visitor->visit_dict(this); }
};

}
}

#endif /* _DICT_NODE_ */
