#include <interpreter/error.h>
#include "parse/symtable.h"

using namespace mtpython::parse;
using namespace mtpython::tree;

void Scope::add_child(Scope* child)
{
    child->parent = this;
    children.push_back(child);
}

const std::string& Scope::add_name(const std::string& id, int flags)
{
    auto iter = id2flags.find(id);
    bool found = false;
    if (iter != id2flags.end()) {
        found = true;
        if (flags & SYM_PARAM && iter->second & SYM_PARAM) {
            throw SyntaxError("duplicate argument in function definition");
        }

        flags |= iter->second;
    }

    id2flags[id] = flags;
    if (flags & SYM_PARAM && !found) {
        varnames.push_back(id);
    }

    return id;
}

void Scope::note_return(mtpython::tree::ReturnNode* node)
{
    throw interpreter::InterpError(
        space->SyntaxError_type(),
        space->wrap_str(space->current_thread(), "return outside function"));
}

void Scope::note_yield(mtpython::tree::YieldNode* node)
{
    throw interpreter::InterpError(
        space->SyntaxError_type(),
        space->wrap_str(space->current_thread(), "yield outside function"));
}

int Scope::lookup(const std::string& id)
{
    auto got = symbols.find(id);
    if (got == symbols.end()) return SCOPE_UNKNOWN;
    return got->second;
}

void Scope::analyze_name(const std::string& id, int flags,
                         std::unordered_set<std::string>& local,
                         std::unordered_set<std::string>& bound,
                         std::unordered_set<std::string>& free,
                         std::unordered_set<std::string>& global)
{
    if (flags & SYM_BOUND) {
        symbols[id] = SCOPE_LOCAL;
        local.insert(id);
        global.erase(id);
    } else if (bound.find(id) != bound.end()) {
        symbols[id] = SCOPE_FREE;
        has_free = true;
        free.insert(id);
        free_vars.push_back(id);
    } else if (global.find(id) != global.end()) {
        symbols[id] = SCOPE_GLOBAL_IMPLICIT;
    } else {
        symbols[id] = SCOPE_GLOBAL_IMPLICIT;
    }
}

void Scope::analyze_cells(std::unordered_set<std::string>& free) {}

void Scope::pass_on_bindings(const std::unordered_set<std::string>& local,
                             const std::unordered_set<std::string>& bound,
                             const std::unordered_set<std::string>& global,
                             std::unordered_set<std::string>& new_bound,
                             std::unordered_set<std::string>& new_global)
{
    new_bound.insert(bound.begin(), bound.end());
    new_global.insert(global.begin(), global.end());
}

void Scope::analyze(std::unordered_set<std::string>& bound,
                    std::unordered_set<std::string>& free,
                    std::unordered_set<std::string>& global)
{
    symbols.clear();

    std::unordered_set<std::string> new_bound, new_free, new_global, local;

    /* class scope has no effects on names visible in nested functions */
    if (is_class_scope) {
        pass_on_bindings(local, bound, global, new_bound, new_global);
    }

    for (auto iter = id2flags.begin(); iter != id2flags.end(); iter++) {
        analyze_name(iter->first, iter->second, local, bound, free, global);
    }

    if (!is_class_scope) {
        pass_on_bindings(local, bound, global, new_bound, new_global);
    } else {
        new_bound.insert("@__class__");
    }

    std::unordered_set<std::string> allfree;
    for (auto child : children) {
        std::unordered_set<std::string> bound_copy(new_bound),
            child_free(new_free), global_copy(global);
        child->analyze(bound_copy, child_free, global_copy);
        allfree.insert(child_free.begin(), child_free.end());
        if (child->has_free || child->child_has_free) child_has_free = true;
    }

    new_free.insert(allfree.begin(), allfree.end());
    analyze_cells(new_free);

    for (const auto& name : new_free) {
        int flags;
        auto iter = id2flags.find(name);

        if (iter == id2flags.end()) {
            if (bound.find(name) != bound.end()) {
                symbols[name] = SCOPE_FREE;
                free_vars.push_back(name);
            }
        } else {
            flags = iter->second;
            if ((flags & (SYM_BOUND | SYM_GLOB)) && is_class_scope) {
                free_vars.push_back(name);
            }
        }
    }

    free.insert(new_free.begin(), new_free.end());
}

void FunctionScope::analyze_cells(std::unordered_set<std::string>& free)
{
    for (auto iter = symbols.begin(); iter != symbols.end(); iter++) {
        if (iter->second == SCOPE_LOCAL &&
            free.find(iter->first) != free.end()) {
            iter->second = SCOPE_CELL;
            free.erase(iter->first);
        }
    }
}

const std::string& FunctionScope::add_name(const std::string& id, int flags)
{
    if (id == "super" && flags == SYM_USE) {
        add_name("@__class__", SYM_USE);
    }

    return Scope::add_name(id, flags);
}

void FunctionScope::pass_on_bindings(
    const std::unordered_set<std::string>& local,
    const std::unordered_set<std::string>& bound,
    const std::unordered_set<std::string>& global,
    std::unordered_set<std::string>& new_bound,
    std::unordered_set<std::string>& new_global)
{
    new_bound.insert(local.begin(), local.end());
    Scope::pass_on_bindings(local, bound, global, new_bound, new_global);
}

void FunctionScope::note_return(mtpython::tree::ReturnNode* node)
{
    if (node->get_value()) {
        if (_is_generator) {
            throw interpreter::InterpError(
                space->SyntaxError_type(),
                space->wrap_str(space->current_thread(),
                                "'return' with argument in generator"));
        }
        return_with_value = true;
    }
}

void FunctionScope::note_yield(mtpython::tree::YieldNode* node)
{
    if (return_with_value) {
        throw interpreter::InterpError(
            space->SyntaxError_type(),
            space->wrap_str(space->current_thread(),
                            "'return' with argument in generator"));
    }
    _is_generator = true;
}

void ClassScope::analyze_cells(std::unordered_set<std::string>& free)
{
    for (auto iter = symbols.begin(); iter != symbols.end(); iter++) {
        if (iter->second == SCOPE_LOCAL &&
            free.find(iter->first) != free.end() &&
            iter->first == "@__class__") {
            iter->second = SCOPE_CELL;
            free.erase(iter->first);
        }
    }
}

SymtableVisitor::SymtableVisitor(mtpython::objects::ObjSpace* space,
                                 ASTNode* module)
{
    this->space = space;

    std::string root_name("root");
    root = new ModuleScope(space, root_name);
    push_scope(root, module);
    current = root;

    module->visit(this);
    std::unordered_set<std::string> bound, free, global;
    root->analyze(bound, free, global);
}

Scope* SymtableVisitor::find_scope(ASTNode* node)
{
    std::unordered_map<mtpython::tree::ASTNode*, Scope*>::const_iterator got =
        scopes.find(node);

    return (got == scopes.end()) ? nullptr : got->second;
}

void SymtableVisitor::push_scope(Scope* scope, ASTNode* node)
{
    if (!stack.empty()) {
        Scope* parent = stack.top();
        parent->add_child(scope);
    }

    stack.push(scope);
    scopes[node] = scope;
    current = scope;
}

void SymtableVisitor::pop_scope()
{
    stack.pop();
    if (!stack.empty())
        current = stack.top();
    else
        current = nullptr;
}

void SymtableVisitor::add_name(const std::string& id, int flags)
{
    const std::string& name = current->add_name(id, flags);

    if (flags & SYM_GLOB) root->add_name(id, flags);
}

ASTNode* SymtableVisitor::visit_alias(AliasNode* node)
{
    std::string name = node->get_asname();
    if (name == "") {
        name = node->get_name();
        if (name == "*") return node;

        std::size_t dot = name.find('.');
        if (dot != std::string::npos) name = name.substr(0, dot);
    }

    add_name(name, SYM_ASSIGN);
    return node;
}

ASTNode* SymtableVisitor::visit_classdef(ClassDefNode* node)
{
    add_name(node->get_name(), SYM_ASSIGN);
    std::vector<ASTNode*>& bases = node->get_bases();
    for (auto& base : bases) {
        base->visit(this);
    }
    std::vector<KeywordNode*>& keywords = node->get_keywords();
    for (auto& keyword : keywords) {
        keyword->visit(this);
    }

    ClassScope* scope = new ClassScope(space, node);
    push_scope(scope, node);

    scope->add_name("@__class__", SYM_ASSIGN);
    visit_sequence(node->get_body());
    pop_scope();

    return node;
}

ASTNode* SymtableVisitor::visit_functiondef(FunctionDefNode* node)
{
    add_name(node->get_name(), SYM_ASSIGN);

    FunctionScope* scope =
        new FunctionScope(space, node->get_name(), node->get_line(), 0);
    push_scope(scope, node);
    node->get_args()->visit(this);
    visit_sequence(node->get_body());
    pop_scope();

    return node;
}

ASTNode* SymtableVisitor::visit_lambda(LambdaNode* node)
{
    FunctionScope* scope =
        new FunctionScope(space, "lambda", node->get_line(), 0);
    push_scope(scope, node);
    node->get_args()->visit(this);
    node->get_body()->visit(this);
    pop_scope();

    return node;
}

ASTNode* SymtableVisitor::visit_name(NameNode* node)
{
    add_name(node->get_name(),
             node->get_context() == EC_LOAD ? SYM_USE : SYM_ASSIGN);

    return node;
}

ASTNode* SymtableVisitor::visit_arguments(ArgumentsNode* node)
{
    std::vector<ASTNode*>& args = node->get_args();
    for (unsigned int i = 0; i < args.size(); i++) {
        NameNode* name = dynamic_cast<NameNode*>(args[i]);
        if (name) {
            add_name(name->get_name(), SYM_PARAM);
        }
    }

    return node;
}

ASTNode* SymtableVisitor::visit_excepthandler(ExceptHandlerNode* node)
{
    std::string& name = node->get_name();
    if (name != "") add_name(name, SYM_ASSIGN);

    visit_sequence(node->get_body());
    if (ASTNode* type = node->get_type()) type->visit(this);

    return node;
}

mtpython::tree::ASTNode*
SymtableVisitor::visit_return(mtpython::tree::ReturnNode* node)
{
    current->note_return(node);
    return GenericVisitor::visit_return(node);
}

mtpython::tree::ASTNode*
SymtableVisitor::visit_yield(mtpython::tree::YieldNode* node)
{
    current->note_yield(node);
    return GenericVisitor::visit_yield(node);
}
