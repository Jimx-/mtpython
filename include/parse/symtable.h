#ifndef _SYMTABLE_H_
#define _SYMTABLE_H_

#include "tree/visitors/generic_visitor.h"
#include "objects/obj_space.h"
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mtpython {
namespace parse {

#define SYM_USE 1
#define SYM_ASSIGN 2
#define SYM_PARAM 4
#define SYM_GLOB 8
#define SYM_BOUND (SYM_ASSIGN | SYM_PARAM)

#define SCOPE_UNKNOWN 0
#define SCOPE_LOCAL 1
#define SCOPE_GLOBAL_IMPLICIT 2
#define SCOPE_GLOBAL_EXPLICIT 3
#define SCOPE_FREE 4
#define SCOPE_CELL 5

class Scope {
protected:
    objects::ObjSpace* space;

    std::string name;
    int line, col;
    Scope* parent;
    std::vector<Scope*> children;
    bool _can_be_optimized;
    bool is_class_scope;
    bool _optimized;
    bool has_free;
    bool child_has_free;
    bool _is_generator;
    bool has_vararg;
    bool has_kwarg;
    std::vector<std::string> varnames;
    std::vector<std::string> free_vars;
    std::unordered_map<std::string, int> id2flags;

    std::unordered_map<std::string, int> symbols;

    void analyze_name(const std::string& id, int flags,
                      std::unordered_set<std::string>& local,
                      std::unordered_set<std::string>& bound,
                      std::unordered_set<std::string>& free,
                      std::unordered_set<std::string>& global);
    virtual void analyze_cells(std::unordered_set<std::string>& free);

    virtual void pass_on_bindings(const std::unordered_set<std::string>& local,
                                  const std::unordered_set<std::string>& bound,
                                  const std::unordered_set<std::string>& global,
                                  std::unordered_set<std::string>& new_bound,
                                  std::unordered_set<std::string>& new_global);

public:
    Scope(objects::ObjSpace* sp, const std::string& name, int line, int col)
        : name(name), line(line), col(col)
    {
        space = sp;
        _can_be_optimized = false;
        _optimized = true;
        is_class_scope = false;
        child_has_free = false;
        has_free = false;
        has_vararg = false;
        has_kwarg = false;
        _is_generator = false;
    }

    void add_child(Scope* child);
    virtual const std::string& add_name(const std::string& id, int flags);
    virtual void note_return(mtpython::tree::ReturnNode* node);
    virtual void note_yield(mtpython::tree::YieldNode* node);
    int lookup(const std::string& id);

    bool can_be_optimized() { return _can_be_optimized; }
    bool is_generator() { return _is_generator; }
    bool optimized() { return _optimized; }
    std::vector<std::string>& get_varnames() { return varnames; }
    std::vector<std::string>& get_free_vars() { return free_vars; }
    std::unordered_map<std::string, int>& get_symbols() { return symbols; }

    void analyze(std::unordered_set<std::string>& bound,
                 std::unordered_set<std::string>& free,
                 std::unordered_set<std::string>& global);
};

class ModuleScope : public Scope {
public:
    ModuleScope(objects::ObjSpace* space, const std::string& name)
        : Scope(space, name, 0, 0)
    {}
};

class FunctionScope : public Scope {
private:
    bool return_with_value;

protected:
    void analyze_cells(std::unordered_set<std::string>& free);

    void pass_on_bindings(const std::unordered_set<std::string>& local,
                          const std::unordered_set<std::string>& bound,
                          const std::unordered_set<std::string>& global,
                          std::unordered_set<std::string>& new_bound,
                          std::unordered_set<std::string>& new_global);

public:
    FunctionScope(objects::ObjSpace* space, const std::string& name, int line,
                  int col)
        : Scope(space, name, line, col)
    {
        _can_be_optimized = true;
        _optimized = true;
        return_with_value = false;
    }

    const std::string& add_name(const std::string& id, int flags);
    void note_return(mtpython::tree::ReturnNode* node);
    void note_yield(mtpython::tree::YieldNode* node);
};

class ClassScope : public Scope {
protected:
    void analyze_cells(std::unordered_set<std::string>& free);

public:
    ClassScope(objects::ObjSpace* space, mtpython::tree::ClassDefNode* cls)
        : Scope(space, cls->get_name(), cls->get_line(), 0)
    {
        is_class_scope = true;
    }
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
    SymtableVisitor(mtpython::objects::ObjSpace* space,
                    mtpython::tree::ASTNode* module);

    Scope* find_scope(mtpython::tree::ASTNode* node);

    /*virtual ASTNode* visit_module(ModuleNode* node); */
    mtpython::tree::ASTNode* visit_alias(mtpython::tree::AliasNode* node);
    mtpython::tree::ASTNode*
    visit_arguments(mtpython::tree::ArgumentsNode* node);
    /*ASTNode* visit_assign(AssignNode* node);
    ASTNode* visit_augassign(AugAssignNode* node);
    ASTNode* visit_binop(BinOpNode* node);
    ASTNode* visit_break(BreakNode* node);
    ASTNode* visit_compare(CompareNode* node);
    ASTNode* visit_continue(ContinueNode* node);
    ASTNode* visit_delete(DeleteNode* node); */
    mtpython::tree::ASTNode* visit_classdef(mtpython::tree::ClassDefNode* node);
    mtpython::tree::ASTNode*
    visit_excepthandler(mtpython::tree::ExceptHandlerNode* node);
    /*ASTNode* visit_for(ForNode* node);*/
    mtpython::tree::ASTNode*
    visit_functiondef(mtpython::tree::FunctionDefNode* node);
    /*ASTNode* visit_if(IfNode* node);
    ASTNode* visit_ifexp(IfExpNode* node);*/
    mtpython::tree::ASTNode* visit_lambda(mtpython::tree::LambdaNode* node);
    mtpython::tree::ASTNode* visit_name(mtpython::tree::NameNode* node);
    /*ASTNode* visit_number(NumberNode* node);
    ASTNode* visit_pass(PassNode* node);
    ASTNode* visit_raise(RaiseNode* node); */
    mtpython::tree::ASTNode* visit_return(mtpython::tree::ReturnNode* node);
    /*ASTNode* visit_tuple(TupleNode* node);
    ASTNode* visit_unaryop(UnaryOpNode* node);
    ASTNode* visit_while(WhileNode* node); */
    mtpython::tree::ASTNode* visit_yield(mtpython::tree::YieldNode* node);
    /*ASTNode* visit_yieldfrom(YieldFromNode* node);*/
};

} // namespace parse
} // namespace mtpython

#endif /* _SYMTABLE_H_ */
