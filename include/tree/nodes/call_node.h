#ifndef _CALL_NODE_
#define _CALL_NODE_

#include "tree/nodes/node.h"
#include "tree/nodes/keyword_node.h"
#include "parse/token.h"
#include <iostream>
#include <vector>
#include "macros.h"

namespace mtpython { 
namespace tree {

class CallNode : public ASTNode {
private:
	ASTNode* func;

	std::vector<ASTNode*> args;
	std::vector<KeywordNode*> keywords;
public:
	CallNode(const int line_nr);
	~CallNode()
	{
		SAFE_DELETE(func);
		for (std::size_t i = 0; i < args.size(); i++) SAFE_DELETE(args[i]);
		for (std::size_t i = 0; i < keywords.size(); i++) SAFE_DELETE(keywords[i]);
	}

	ASTNode* get_func() { return this->func; }
	void set_func(ASTNode* func) { this->func = func; }
	void push_arg(ASTNode* arg) { args.push_back(arg); }
	std::vector<ASTNode*>& get_args() { return args; }
	void push_keyword(KeywordNode* keyword) { keywords.push_back(keyword); }
	std::vector<KeywordNode*>& get_keywords() { return keywords; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Call: "<< std::endl;
		std::cout << blank << "  " << line << ": Func: " << std::endl;
		func->print(padding + 4);
		std::cout << blank << "  " << line << ": Args: " << std::endl;
		for (unsigned int i = 0; i < args.size(); i++) {
			args[i]->print(padding + 4);
		}
		std::cout << blank << "  " << line << ": Keywords: " << std::endl;
		for (unsigned int i = 0; i < keywords.size(); i++) {
			keywords[i]->print(padding + 4);
		}
	}
	
	virtual NodeType get_tag() { return NT_CALL; }

	virtual void visit(ASTVisitor* visitor) { visitor->visit_call(this); }
};

}
}

#endif /* _CALL_NODE_ */
