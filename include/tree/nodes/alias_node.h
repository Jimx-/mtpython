#ifndef _ALIAS_NODE_
#define _ALIAS_NODE_

#include "tree/nodes/node.h"
#include "parse/token.h"
#include <iostream>
#include <string>
#include "macros.h"

namespace mtpython {
namespace tree {

class AliasNode : public ASTNode {
private:
	std::string name;
	std::string asname;

public:
	AliasNode(const int line_nr);
	~AliasNode() { }

	std::string& get_name() { return name; }
	void set_name(const std::string& name) { this->name = name; }
	std::string& get_asname() { return asname; }
	void set_asname(const std::string& asname) { this->asname = asname; }

	virtual void print(const int padding) {
		std::string blank(padding, ' ');
		std::cout << blank << line << ": Alias: " << std::endl;
		std::cout << blank << "  " << line << ": Name: " << name << std::endl;
		std::cout << blank << "  " << line << ": Asname:" << asname << std::endl;
	}
	virtual NodeType get_tag() { return NT_ALIAS; }
	virtual void visit(ASTVisitor* visitor) { visitor->visit_alias(this); }
};

}
}

#endif /* _ALIAS_NODE_ */
