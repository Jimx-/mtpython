#ifndef _SYMTABLE_H_
#define _SYMTABLE_H_

#include "tree/visitors/generic_visitor.h"
#include "objects/obj_space.h"
#include <stack>
#include <unordered_map>
#include <vector>

namespace mtpython {
namespace parse {

#define SYM_USE			1
#define SYM_ASSIGN		2
#define SYM_PARAM		4
#define SYM_GLOB		8
#define SYM_BOUND		(SYM_ASSIGN | SYM_PARAM)

#define SCOPE_UNKNOWN	0
#define SCOPE_LOCAL		1

class Scope {
protected:
	std::string name;
	int line, col;
	Scope* parent;
	std::vector<Scope*> children;
	std::vector<std::string> varnames;
	std::unordered_map<std::string, int> id2flags;

	std::unordered_map<std::string, int> symbols;

	void finalize_name(const std::string& id, int flags);

public:
	Scope(std::string& name, int line, int col) : name(name), line(line), col(col) { }

	void add_child(Scope* child);
	std::string& add_name(std::string& id, int flags);
	int lookup(std::string& id);

	std::vector<std::string>& get_varnames() { return varnames; }

	void finalize();
};

class ModuleScope : public Scope {
public:
	ModuleScope(std::string& name) : Scope(name, 0, 0) { }
};

class FunctionScope : public Scope {
public:
	FunctionScope(std::string& name, int line, int col) : Scope(name, line, col) { }
};

class SymtableVisitor : public mtpython::tree::GenericVisitor {
private:
	std::stack<Scope*> stack;
	mtpython::objects::ObjSpace* space;
	Scope* root;
	Scope* current;
	std::unordered_map<mtpython::tree::ASTNode*, Scope*> scopes;

	void push_scope(Scope* scope, mtpython::tree::ASTNode* node);
	void pop_scope();

	void add_name(std::string& id, int flags);

public:
	SymtableVisitor(mtpython::objects::ObjSpace* space, mtpython::tree::ASTNode* module);

	Scope* find_scope(mtpython::tree::ASTNode* node);

	/*virtual ASTNode* visit_module(ModuleNode* node); */
    virtual mtpython::tree::ASTNode* visit_arguments(mtpython::tree::ArgumentsNode* node);
    /*virtual ASTNode* visit_assign(AssignNode* node);
    virtual ASTNode* visit_augassign(AugAssignNode* node);
    virtual ASTNode* visit_binop(BinOpNode* node);
    virtual ASTNode* visit_break(BreakNode* node);
    virtual ASTNode* visit_compare(CompareNode* node);
    virtual ASTNode* visit_continue(ContinueNode* node);
    virtual ASTNode* visit_delete(DeleteNode* node);
    virtual ASTNode* visit_for(ForNode* node);*/
    virtual mtpython::tree::ASTNode* visit_functiondef(mtpython::tree::FunctionDefNode* node);
    /*virtual ASTNode* visit_if(IfNode* node);
    virtual ASTNode* visit_ifexp(IfExpNode* node);*/
    virtual mtpython::tree::ASTNode* visit_name(mtpython::tree::NameNode* node);
    /*virtual ASTNode* visit_number(NumberNode* node);
    virtual ASTNode* visit_pass(PassNode* node);
    virtual ASTNode* visit_raise(RaiseNode* node);
    virtual ASTNode* visit_return(ReturnNode* node);
    virtual ASTNode* visit_tuple(TupleNode* node);
    virtual ASTNode* visit_unaryop(UnaryOpNode* node);
    virtual ASTNode* visit_while(WhileNode* node);
    virtual ASTNode* visit_yield(YieldNode* node);
    virtual ASTNode* visit_yieldfrom(YieldFromNode* node);*/
};

}
}

#endif	/* _SYMTABLE_H_ */
