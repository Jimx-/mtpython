#ifndef _NODE_H_
#define _NODE_H_

#include <cstddef>
#include "exceptions.h"

namespace mtpython {
namespace tree {

#define error_tree NULL

typedef enum {
	NT_EMPTY, NT_IF, NT_FOR, NT_FOREACH, NT_ASSIGN, NT_WHILE, NT_DOWHILE, NT_TRY,
	NT_CATCH, NT_FINALLY, NT_CASE, NT_RETURN, NT_BREAK, NT_CONTINUE, NT_EXPRSTMT,
	NT_WHEN, NT_IDENT, NT_UNARY, NT_BINARY, NT_LITERAL, NT_ARGUMENTS, NT_FUNCCALL,
    NT_MODULE,
} NodeType;

class ASTNode {
protected:
	int line;					// line nr
	
	ASTNode* sibling;		// sibling node
	
public:
    ASTNode(const int line_nr) : line(line_nr) { sibling = NULL; }
    ~ASTNode() { }

    int get_line() { return line; }
    ASTNode * get_sibling() { return this->sibling; }
    void set_sibling(ASTNode * sbl) { this->sibling = sbl; }
    virtual NodeType get_tag() { return NT_EMPTY; }
    virtual void print(const int padding) {}

    int length() 
    {
    	int ret = 0;
    	ASTNode * tree = this;
    	while (tree) {
    		ret ++;
    		tree = tree->sibling;
    	}
    	return ret;
    }
};

}
}

#endif /* _NODE_H_ */
