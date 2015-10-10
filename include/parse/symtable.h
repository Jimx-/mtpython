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
#define SCOPE_GLOBAL_IMPLICIT	2

class Scope {
protected:
	std::string name;
	int line, col;
	Scope* parent;
	std::vector<Scope*> children;
	bool _can_be_optimized;
	std::vector<std::string> varnames;
	std::unordered_map<std::string, int> id2flags;

	std::unordered_map<std::string, int> symbols;

	void finalize_name(const std::string& id, int flags);

public:
	Scope(const std::string& name, int line, int col) : name(name), line(line), col(col) { _can_be_optimized = false; }

	void add_child(Scope* child);
	const std::string& add_name(const std::string& id, int flags);
	int lookup(const std::string& id);

	bool can_be_optimized() { return _can_be_optimized; }
	std::vector<std::string>& get_varnames() { return varnames; }

	void finalize();
};

class ModuleScope : public Scope {
public:
	ModuleScope(const std::string& name) : Scope(name, 0, 0) { }
};

class FunctionScope : public Scope {
public:
	FunctionScope(const std::string& name, int line, int col) : Scope(name, line, col) { _can_be_optimized = true; }
};

class ClassScope : public Scope {
public:
	ClassScope(mtpython::tree::ClassDefNode* cls) : Scope(cls->get_name(), cls->get_line(), 0) { }
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

	void add_name(const std::string& id, int flags);

public:
	SymtableVisitor(mtpython::objects::ObjSpace* space, mtpython::tree::ASTNode* module);

	Scope* find_scope(mtpython::tree::ASTNode* node);

	/*virtual ASTNode* visit_module(ModuleNode* node); */
	mtpython::tree::ASTNode* visit_alias(mtpython::tree::AliasNode* node);
    mtpython::tree::ASTNode* visit_arguments(mtpython::tree::ArgumentsNode* node);
    /*ASTNode* visit_assign(AssignNode* node);
    ASTNode* visit_augassign(AugAssignNode* node);
    ASTNode* visit_binop(BinOpNode* node);
    ASTNode* visit_break(BreakNode* node);
    ASTNode* visit_compare(CompareNode* node);
    ASTNode* visit_continue(ContinueNode* node);
    ASTNode* visit_delete(DeleteNode* node); */
	mtpython::tree::ASTNode* visit_classdef(mtpython::tree::ClassDefNode* node);
	mtpython::tree::ASTNode* visit_excepthandler(mtpython::tree::ExceptHandlerNode* node);
    /*ASTNode* visit_for(ForNode* node);*/
    mtpython::tree::ASTNode* visit_functiondef(mtpython::tree::FunctionDefNode* node);
    /*ASTNode* visit_if(IfNode* node);
    ASTNode* visit_ifexp(IfExpNode* node);*/
	mtpython::tree::ASTNode* visit_lambda(mtpython::tree::LambdaNode* node);
    mtpython::tree::ASTNode* visit_name(mtpython::tree::NameNode* node);
    /*ASTNode* visit_number(NumberNode* node);
    ASTNode* visit_pass(PassNode* node);
    ASTNode* visit_raise(RaiseNode* node);
    ASTNode* visit_return(ReturnNode* node);
    ASTNode* visit_tuple(TupleNode* node);
    ASTNode* visit_unaryop(UnaryOpNode* node);
    ASTNode* visit_while(WhileNode* node);
    ASTNode* visit_yield(YieldNode* node);
    ASTNode* visit_yieldfrom(YieldFromNode* node);*/
};

}
}

#endif	/* _SYMTABLE_H_ */
