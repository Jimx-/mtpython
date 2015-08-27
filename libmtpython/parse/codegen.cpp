#include "parse/codegen.h"

using namespace mtpython::parse;
using namespace mtpython::interpreter;
using namespace mtpython::tree;
using namespace mtpython::objects;

static std::unordered_map<int, int> name_ops_default = {{EC_LOAD, LOAD_NAME}, {EC_STORE, STORE_NAME}, {EC_DEL, DELETE_NAME}};
static std::unordered_map<int, int> name_ops_fast = {{EC_LOAD, LOAD_FAST}, {EC_STORE, STORE_FAST}, {EC_DEL, DELETE_FAST}};
static std::unordered_map<int, int> name_ops_global = {{EC_LOAD, LOAD_GLOBAL}, {EC_STORE, STORE_GLOBAL}, {EC_DEL, DELETE_GLOBAL}};

BaseCodeGenerator::BaseCodeGenerator(const std::string& name, ObjSpace* space, ASTNode* module, SymtableVisitor* symtab, int lineno, CompileInfo* info) : CodeBuilder(name, space, symtab->find_scope(module), lineno, info)
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

ASTNode* BaseCodeGenerator::visit_expr(ExprNode* node)
{
	set_lineno(node->get_line());
	ASTNode* value = node->get_value();

	if ((value->get_tag() != NodeType::NT_NUMBER) && (value->get_tag() != NodeType::NT_STRING)) {
		value->visit(this);
		emit_op(POP_TOP);
	}

	return node;
}

void BaseCodeGenerator::gen_name(const std::string& name, ExprContext ctx)
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

static int compare_operation(CmpOper op)
{
	switch (op) {
	case OP_LSS:
		return 0;
	case OP_GTR:
		return 4;
	case OP_LSSEQ:
		return 1;
	case OP_GTREQ:
		return 5;
	case OP_EQ:
		return 2;
	case OP_NOTEQ:
		return 3;
	}

	return 0;
}

ASTNode* BaseCodeGenerator::visit_compare(CompareNode *node) {
	set_lineno(node->get_line());

	node->get_left()->visit(this);
	std::vector<CmpOper>& ops = node->get_ops();
	int opcount = ops.size();
	CodeBlock* cleanup = nullptr;

	std::vector<ASTNode*>& comparators = node->get_comparators();

	if (opcount > 1) {
		cleanup = new_block();
		comparators[0]->visit(this);
	}
	for (unsigned int i = 1; i < opcount; i++) {
		emit_op(DUP_TOP);
		emit_op(ROT_THREE);
		int arg = compare_operation(ops[i - 1]);
		emit_op_arg(COMPARE_OP, arg);
		emit_jump(JUMP_IF_FALSE_OR_POP, cleanup, true);
		if (i < opcount - 1) comparators[i]->visit(this);
	}

	CmpOper last_op = ops.back();
	ASTNode* last_comparator = comparators.back();

	last_comparator->visit(this);
	emit_op_arg(COMPARE_OP, compare_operation(last_op));

	if (opcount > 1) {
		CodeBlock* end = new_block();
		emit_jump(JUMP_FORWARD, end);
		use_next_block(cleanup);
		emit_op(ROT_TWO);
		emit_op(POP_TOP);
		use_next_block(end);
	}

	return node;
}

void BaseCodeGenerator::make_closure(mtpython::interpreter::PyCode* code, int args, mtpython::objects::M_BaseObject* qualname)
{
	int nfree = code->get_nfreevars();

	if (!qualname) {
		qualname = space->wrap_str(code->get_name());
	}

	if (nfree == 0) {
		load_const(code);
		load_const(qualname);
		emit_op_arg(MAKE_FUNCTION, args);
	}
}

ASTNode* BaseCodeGenerator::visit_functiondef(FunctionDefNode* node)
{
	set_lineno(node->get_line());
	FunctionCodeGenerator sub_gen(node->get_name(), space, node, symtab, node->get_line(), compile_info);
	PyCode* code = sub_gen.build();

	int arglength = 0;

	make_closure(code, arglength, get_qualname());
	
	gen_name(node->get_name(), ExprContext::EC_STORE);
	return node;
}

ASTNode* BaseCodeGenerator::visit_if(IfNode* node)
{
	set_lineno(node->get_line());
	CodeBlock* end_block = new_block();

	ASTNode* orelse = node->get_orelse();
	CodeBlock* otherwise = orelse ? new_block() : end_block;

	int constant = expr_constant(node->get_test());

	if (constant == 0) {
		if (orelse) orelse->visit(this);
	} else if (constant == 1) {
		node->get_body()->visit(this);
	} else {
		node->get_test()->visit(this);
		emit_jump(POP_JUMP_IF_FALSE, otherwise, true);
		node->get_body()->visit(this);
		emit_jump(JUMP_FORWARD, end_block);

		if (orelse) {
			use_next_block(otherwise);
			orelse->visit(this);
		}
	}

	use_next_block(end_block);

	return node;
}

ASTNode* BaseCodeGenerator::visit_name(NameNode* node)
{
	set_lineno(node->get_line());
	gen_name(node->get_name(), node->get_context());

	return node;
}

ASTNode* BaseCodeGenerator::visit_keyword(KeywordNode* node)
{
	load_const(space->wrap_str(node->get_arg()));
	node->get_value()->visit(this);

	return node;
}

ASTNode* BaseCodeGenerator::visit_number(NumberNode* node)
{
	set_lineno(node->get_line());
	load_const(node->get_value());

	return node;
}

ASTNode* BaseCodeGenerator::visit_return(ReturnNode* node)
{
	set_lineno(node->get_line());
	ASTNode* value = node->get_value();
	if (value) {
		value->visit(this);
	} else {
		load_const(space->wrap_None());
	}

	emit_op(RETURN_VALUE);

	return node;
}

ASTNode* BaseCodeGenerator::visit_string(StringNode* node)
{
	set_lineno(node->get_line());
	load_const(node->get_value());

	return node;
}

ModuleCodeGenerator::ModuleCodeGenerator(mtpython::objects::ObjSpace* space, mtpython::tree::ASTNode* module, SymtableVisitor* symtab, CompileInfo* info) : BaseCodeGenerator("module", space, module, symtab, -1, info)
{
	compile(module);
}

void ModuleCodeGenerator::compile(ASTNode* module)
{
	module->visit(this);
}

FunctionCodeGenerator::FunctionCodeGenerator(const std::string& name, mtpython::objects::ObjSpace* space, mtpython::tree::ASTNode* tree, SymtableVisitor* symtab, int lineno, CompileInfo* info) : BaseCodeGenerator(name, space, tree, symtab, lineno, info)
{
	compile(tree);
}

void FunctionCodeGenerator::compile(ASTNode* tree)
{
	FunctionDefNode* funcdef = dynamic_cast<FunctionDefNode*>(tree);
	ArgumentsNode* arguments = dynamic_cast<ArgumentsNode*>(funcdef->get_args());

	set_argcount(arguments->get_args().size());
	ASTNode* body = funcdef->get_body();

	if (body) visit_sequence(body);
}

