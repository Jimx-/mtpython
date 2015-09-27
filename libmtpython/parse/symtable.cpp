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

int Scope::lookup(const std::string& id)
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

	for (auto child : children) {
		child->finalize();
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

ASTNode* SymtableVisitor::visit_lambda(LambdaNode* node)
{
	FunctionScope* scope = new FunctionScope("lambda", node->get_line(), 0);
	push_scope(scope, node);
	node->get_args()->visit(this);
	node->get_body()->visit(this);
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

ASTNode* SymtableVisitor::visit_excepthandler(ExceptHandlerNode* node)
{
	std::string& name = node->get_name();
	if (name != "") add_name(name, SYM_ASSIGN);

	visit_sequence(node->get_body());
	if (ASTNode* type = node->get_type()) type->visit(this);

	return node;
}
