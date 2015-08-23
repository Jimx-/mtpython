#include "parse/symtable.h"
#include "exceptions.h"
#include <iostream>

using namespace mtpython::parse;
using namespace mtpython::tree;

void Scope::add_child(Scope* child)
{
	child->parent = this;
	children.push_back(child);
}

std::string& Scope::add_name(std::string& id, int flags)
{
	auto iter = id2flags.find(id);
	if (iter != id2flags.end()) {
		if (flags & SYM_PARAM && iter->second * SYM_PARAM) {
			throw SyntaxError("duplicate argument in function definition");
		}

		flags |= iter->second;
	}

	id2flags[id] = flags;
	if (flags & SYM_PARAM) varnames.push_back(id);

	return id;
}

int Scope::lookup(std::string& id)
{
	auto got = symbols.find(id);
	if (got == symbols.end()) return SCOPE_UNKNOWN;
	return got->second;
}

void Scope::finalize_name(const std::string& id, int flags)
{
	if (flags & SYM_BOUND) {
		symbols[id] = SCOPE_LOCAL;
	} else
		symbols[id] = SCOPE_GLOBAL_IMPLICIT;
}

void Scope::finalize()
{
	symbols.clear();

	for (auto iter = id2flags.begin(); iter != id2flags.end(); iter++) {
		finalize_name(iter->first, iter->second);
	}
}

SymtableVisitor::SymtableVisitor(mtpython::objects::ObjSpace* space, ASTNode* module)
{
	this->space = space;

	std::string root_name("root");
	root = new ModuleScope(root_name);
	push_scope(root, module);
	current = root;

	module->visit(this);
	root->finalize();
}

Scope* SymtableVisitor::find_scope(ASTNode* node)
{
	std::unordered_map<mtpython::tree::ASTNode*, Scope*>::const_iterator got = scopes.find(node);

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

void SymtableVisitor::add_name(std::string& id, int flags)
{
	std::string& name = current->add_name(id, flags);

	if (flags & SYM_GLOB) root->add_name(id, flags);
}

ASTNode* SymtableVisitor::visit_functiondef(FunctionDefNode* node)
{
	add_name(node->get_name(), SYM_ASSIGN);

	FunctionScope* scope = new FunctionScope(node->get_name(), node->get_line(), 0);
	push_scope(scope, node);
	node->get_args()->visit(this);
	visit_sequence(node->get_body());
	pop_scope();

	return node;
}

ASTNode* SymtableVisitor::visit_name(NameNode* node)
{
	add_name(node->get_name(), node->get_context() == EC_LOAD ? SYM_USE : SYM_ASSIGN);

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
