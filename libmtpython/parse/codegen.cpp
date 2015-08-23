#include "parse/codegen.h"
#include "parse/token.h"
#include "tools/opcode.h"

using namespace mtpython::parse;
using namespace mtpython::tree;
using namespace mtpython::objects;

static std::unordered_map<int, int> name_ops_default = {{EC_LOAD, LOAD_NAME}, {EC_STORE, STORE_NAME}, {EC_DEL, DELETE_NAME}};
static std::unordered_map<int, int> name_ops_fast = {{EC_LOAD, LOAD_FAST}, {EC_STORE, STORE_FAST}, {EC_DEL, DELETE_FAST}};
static std::unordered_map<int, int> name_ops_global = {{EC_LOAD, LOAD_GLOBAL}, {EC_STORE, STORE_GLOBAL}, {EC_DEL, DELETE_GLOBAL}};

BaseCodeGenerator::BaseCodeGenerator(std::string& name, ObjSpace* space, ASTNode* module, SymtableVisitor* symtab, int lineno, CompileInfo* info) : CodeBuilder(name, space, symtab->find_scope(module), lineno, info)
{
	this->scope = symtab->find_scope(module);
	this->symtab = symtab;
}

char BaseCodeGenerator::_binop(BinaryOper op)
{
	switch (op) {
	case OP_PLUS:
		return BINARY_ADD;
	case OP_MINUS:
		return BINARY_SUBTRACT;
	case OP_MUL:
		return BINARY_MULTIPLY;
	}

	return NOP;
}

ASTNode* BaseCodeGenerator::visit_assign(AssignNode* node)
{
	set_lineno(node->get_line());
	node->get_value()->visit(this);
	std::vector<ASTNode*>& targets = node->get_targets();

	if (targets.size() > 1) emit_op(UNPACK_SEQUENCE);

	for (auto target = targets.begin(); target != targets.end(); target++) {
		(*target)->visit(this);
	}

	return node;
}

ASTNode* BaseCodeGenerator::visit_binop(BinOpNode* node)
{
	set_lineno(node->get_line());
	node->get_left()->visit(this);
	node->get_right()->visit(this);
	emit_op(_binop(node->get_op()));
	
	return node;
}

ASTNode* BaseCodeGenerator::visit_call(CallNode* node)
{
	set_lineno(node->get_line());
	node->get_func()->visit(this);
	std::vector<ASTNode*>& func_args = node->get_args();
	
	int arg = func_args.size();
	for (auto func_arg : func_args) func_arg->visit(this);

	std::vector<KeywordNode*>& keywords = node->get_keywords();
	int call_type = 0;
	if (keywords.size() > 0) {
		for (auto keyword : keywords) keyword->visit(this);
		arg |= keywords.size() << 8;
	}

	unsigned char op = 0;
	if (call_type == 0) op = (unsigned char)CALL_FUNCTION;

	emit_op_arg(op, arg);

	return node;
}

void BaseCodeGenerator::gen_name(std::string& name, ExprContext ctx)
{
	int scope = this->scope->lookup(name);
	char op = NOP;
	int arg = -1;

	if (scope == SCOPE_LOCAL) {
		op = name_ops_fast[ctx];
		arg = add_name(varnames, name);
	} else if (scope == SCOPE_GLOBAL_IMPLICIT) {
		op = name_ops_global[ctx];
		arg = add_name(names, name);
	}

	if (op == NOP) {
		op = name_ops_default[ctx];
		arg = add_name(names, name);
	}

	emit_op_arg(op, arg);
}

ASTNode* BaseCodeGenerator::visit_name(NameNode* node)
{
	set_lineno(node->get_line());
	gen_name(node->get_name(), node->get_context());

	return node;
}

ASTNode* BaseCodeGenerator::visit_number(NumberNode* node)
{
	set_lineno(node->get_line());
	load_const(node->get_value());

	return node;
}

ASTNode* BaseCodeGenerator::visit_string(StringNode* node)
{
	set_lineno(node->get_line());
	load_const(node->get_value());

	return node;
}

ModuleCodeGenerator::ModuleCodeGenerator(mtpython::objects::ObjSpace* space, mtpython::tree::ASTNode* module, SymtableVisitor* symtab, CompileInfo* info) : BaseCodeGenerator(std::string("module"), space, module, symtab, -1, info)
{
	compile(module);
}

void ModuleCodeGenerator::compile(ASTNode* module)
{
	module->visit(this);
}
