#ifndef _ARGUMENTS_NODE_
#define _ARGUMENTS_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython { 
namespace tree {

class ArgumentsNode : public ASTNode {
private:
	std::vector<ASTNode*> args;
	std::vector<ASTNode*> defaults;
public:
	ArgumentsNode(const int line_nr);
	~ArgumentsNode()
	{
		for (std::size_t i = 0; i < args.size(); i++) SAFE_DELETE(args[i]);
		for (std::size_t i = 0; i < args.size(); i++) SAFE_DELETE(defaults[i]);
	}

	std::vector<ASTNode*>& get_args() { return args; }
	void push_arg(ASTNode* arg) { args.push_back(arg); }
	std::vector<ASTNode*>& get_defaults() { return defaults; }
	void push_default(ASTNode* _default) { defaults.push_back(_default); }
	virtual NodeType get_tag() { return NT_ARGUMENTS; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Arguments:" << std::endl;
		std::cout << blank << "  " << line << ": Args:" << std::endl;
		for (unsigned int i = 0; i < args.size(); i++) args[i]->print(padding + 4);

		if (defaults.size()) {
			std::cout << blank << "  " << line << ": Defaults:" << std::endl;
			for (unsigned int i = 0; i < defaults.size(); i++) defaults[i]->print(padding + 4);
		}
	}

	virtual void visit(ASTVisitor* visitor) { visitor->visit_arguments(this); }
};

}
}

#endif /* _ARGUMENTS_NODE_ */
