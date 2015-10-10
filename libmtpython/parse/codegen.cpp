#include "vm/vm.h"
#include "parse/codegen.h"

using namespace mtpython::parse;
using namespace mtpython::interpreter;
using namespace mtpython::tree;
using namespace mtpython::objects;

static std::unordered_map<int, int> name_ops_default = {{EC_LOAD, LOAD_NAME}, {EC_STORE, STORE_NAME}, {EC_DEL, DELETE_NAME}};
static std::unordered_map<int, int> name_ops_fast = {{EC_LOAD, LOAD_FAST}, {EC_STORE, STORE_FAST}, {EC_DEL, DELETE_FAST}};
static std::unordered_map<int, int> name_ops_global = {{EC_LOAD, LOAD_GLOBAL}, {EC_STORE, STORE_GLOBAL}, {EC_DEL, DELETE_GLOBAL}};
static std::unordered_map<int, int> subscr_op = {{EC_LOAD, BINARY_SUBSCR}, {EC_STORE, STORE_SUBSCR}, {EC_DEL, DELETE_SUBSCR}};

BaseCodeGenerator::BaseCodeGenerator(const std::string& name, mtpython::vm::ThreadContext* context, ASTNode* module, SymtableVisitor* symtab, int lineno, CompileInfo* info) : CodeBuilder(name, context, symtab->find_scope(module), lineno, info)
{
	this->scope = symtab->find_scope(module);
	this->symtab = symtab;
}

unsigned char BaseCodeGenerator::_binop(BinaryOper op)
{
	switch (op) {
	case OP_PLUS:
		return BINARY_ADD;
	case OP_MINUS:
		return BINARY_SUBTRACT;
	case OP_MUL:
		return BINARY_MULTIPLY;
	case OP_AND:
		return BINARY_AND;
	}

	return NOP;
}

unsigned char BaseCodeGenerator::_unaryop(UnaryOper op)
{
	switch (op) {
	case OP_POS:
		return UNARY_POSITIVE;
	case OP_NEG:
		return UNARY_NEGATIVE;
	case OP_NOT:
		return UNARY_NOT;
	case OP_BITNOT:
		return UNARY_INVERT;
	}

	return NOP;
}

ASTNode* BaseCodeGenerator::visit_attribute(AttributeNode* node)
{
	set_lineno(node->get_line());
	ExprContext ctx = node->get_context();

	if (ctx != EC_AUGSTORE) node->get_value()->visit(this);

	unsigned char op = NOP;
	if (ctx == EC_LOAD) {
		op = LOAD_ATTR;
	} else if (ctx == EC_STORE) {
		op = STORE_ATTR;
	} else if (ctx == EC_DEL) {
		op = DELETE_ATTR;
	} else if (ctx == EC_AUGLOAD) {
		emit_op(DUP_TOP);
		op = LOAD_ATTR;
	} else if (ctx == EC_AUGSTORE) {
		emit_op(ROT_TWO);
		op = STORE_ATTR;
	}

	emit_op_arg(op, add_name(names, node->get_attr()));

	return node;
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

ASTNode* BaseCodeGenerator::visit_break(BreakNode* node)
{
	set_lineno(node->get_line());
	bool in_loop = false;
	for (auto& fb : frame_block) {
		if (fb.first == FrameType::F_LOOP) in_loop = true;
	}

	if (!in_loop) throw mtpython::SyntaxError("'break' outside loop");

	emit_op(BREAK_LOOP);

	return node;
}

void BaseCodeGenerator::make_call(int n, const std::vector<ASTNode*>& args, const std::vector<KeywordNode*>& keywords)
{
	int arg = args.size() + n;
	for (auto& arg : args) arg->visit(this);

	int call_type = 0;
	if (keywords.size() > 0) {
		for (auto keyword : keywords) keyword->visit(this);
		arg |= keywords.size() << 8;
	}

	unsigned char op = 0;
	if (call_type == 0) op = (unsigned char)CALL_FUNCTION;

	emit_op_arg(op, arg);
}

ASTNode* BaseCodeGenerator::visit_call(CallNode* node)
{
	set_lineno(node->get_line());
	node->get_func()->visit(this);

	make_call(0, node->get_args(), node->get_keywords());

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
		if (this->scope->can_be_optimized()) {
			op = name_ops_fast[ctx];
			arg = add_name(varnames, name);
		}
	} else if (scope == SCOPE_GLOBAL_IMPLICIT) {
		if (this->scope->can_be_optimized()) {
			op = name_ops_global[ctx];
			arg = add_name(names, name);
		}
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
	case OP_IN:
		return 6;
	case OP_NOT_IN:
		return 7;
	case OP_IS:
		return 8;
	case OP_IS_NOT:
		return 9;
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
	for (int i = 1; i < opcount; i++) {
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

ASTNode* BaseCodeGenerator::visit_for(ForNode* node)
{
	set_lineno(node->get_line());
	CodeBlock* start = new_block();
	CodeBlock* cleanup = new_block();
	CodeBlock* end = new_block();

	emit_jump(SETUP_LOOP, end);
	push_frame_block(FrameType::F_LOOP, start);
	node->get_iter()->visit(this);
	emit_op(GET_ITER);

	use_next_block(start);
	emit_jump(FOR_ITER, cleanup);
	node->get_target()->visit(this);
	visit_sequence(node->get_body());
	emit_jump(JUMP_ABSOLUTE, start, true);

	use_next_block(cleanup);
	emit_op(POP_BLOCK);
	pop_frame_block();
	visit_sequence(node->get_orelse());
	use_next_block(end);

	return node;
}

void BaseCodeGenerator::make_closure(mtpython::interpreter::PyCode* code, int args, mtpython::objects::M_BaseObject* qualname)
{
	int nfree = code->get_nfreevars();

	if (!qualname) {
		qualname = space->wrap_str(context, code->get_name());
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
	ASTNode* decorators = node->get_decorators();
	visit_sequence(decorators);

	FunctionCodeGenerator sub_gen(node->get_name(), context, node, symtab, node->get_line(), compile_info);
	PyCode* code = sub_gen.build();

	ArgumentsNode* args = static_cast<ArgumentsNode*>(node->get_args());
	std::vector<ASTNode*> defaults = args->get_defaults();
	for (auto& child : defaults) {
		child->visit(this);
	}

	int arglength = 0;
	arglength |= defaults.size();

	make_closure(code, arglength, get_qualname());

	while (decorators) {
		emit_op_arg(CALL_FUNCTION, 1);
		decorators = decorators->get_sibling();
	}

	gen_name(node->get_name(), ExprContext::EC_STORE);
	return node;
}

ASTNode* BaseCodeGenerator::visit_classdef(ClassDefNode* node)
{
	set_lineno(node->get_line());

	ClassCodeGenerator sub_gen(node->get_name(), context, node, symtab, node->get_line(), compile_info);
	PyCode* code = sub_gen.build();

	emit_op(LOAD_BUILD_CLASS);

	/* __build_class__(func, name, *bases, metaclass=None, **kwds) -> class */
	make_closure(code, 0, nullptr);
	load_const(space->wrap_str(context, node->get_name()));
	make_call(2, node->get_bases(), node->get_keywords());

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
		visit_sequence(node->get_body());
		emit_jump(JUMP_FORWARD, end_block);

		if (orelse) {
			use_next_block(otherwise);
			visit_sequence(orelse);
		}
	}

	use_next_block(end_block);

	return node;
}

ASTNode* BaseCodeGenerator::visit_ifexp(IfExpNode* node)
{
	set_lineno(node->get_line());
	CodeBlock* end_block = new_block();
	CodeBlock* otherwise = new_block();

	node->get_test()->visit(this);
	emit_jump(POP_JUMP_IF_FALSE, otherwise, true);
	node->get_body()->visit(this);
	emit_jump(JUMP_FORWARD, end_block);

	use_next_block(otherwise);
	node->get_orelse()->visit(this);
	use_next_block(end_block);

	return node;
}

ASTNode* BaseCodeGenerator::visit_import(ImportNode* node)
{
	set_lineno(node->get_line());
	for (auto alias : node->get_names()) {
		int level = 0;
		load_const(space->wrap_int(context, level));
		load_const(space->wrap_None());
		emit_op_arg(IMPORT_NAME, add_name(names, alias->get_name()));

		if (alias->get_asname() == "") {
			std::string name = alias->get_name();
			std::size_t dot = name.find('.');
			if (dot != std::string::npos) {
				name = name.substr(0, dot);
			}

			gen_name(name, EC_STORE);
		} else
			import_as(alias);
	}

	return node;
}

ASTNode* BaseCodeGenerator::visit_importfrom(ImportFromNode* node)
{
	set_lineno(node->get_line());
	std::vector<AliasNode*>& names = node->get_names();
	
	std::string first = names[0]->get_name();
	bool star_import = (names.size() == 1) && (first == "*");

	load_const(space->wrap_int(context, node->get_level()));

	std::vector<M_BaseObject*> wrapped_names;
	wrapped_names.resize(names.size(), nullptr);
	for (std::size_t i = 0; i < names.size(); i++) {
		wrapped_names[i] = space->wrap_str(context, names[i]->get_name());
	}

	load_const(space->new_tuple(context, wrapped_names));
	std::string mod_name = node->get_module();

	emit_op_arg(IMPORT_NAME, add_name(this->names, mod_name));
	if (star_import) {
		emit_op(IMPORT_STAR);
	} else {
		for (auto& alias : names) {
			emit_op_arg(IMPORT_FROM, add_name(this->names, alias->get_name()));
			std::string store_name;
			if (alias->get_asname() != "") {
				store_name = alias->get_asname();
			} else {
				store_name = alias->get_name();
			}
			gen_name(store_name, EC_STORE);
		}
		emit_op(POP_TOP);
	}

	return node;
}

void BaseCodeGenerator::import_as(AliasNode* node)
{
	std::string src_name = node->get_name();
	std::size_t dot = src_name.find('.');

	if (dot != std::string::npos) {
		while (true) {
			std::size_t start = dot + 1;
			dot = src_name.find('.', start);
			std::size_t end;

			if (dot != std::string::npos)
				end = dot;
			else
				end = src_name.size();
			std::string attr = src_name.substr(start, end);
			emit_op_arg(LOAD_ATTR, add_name(names, attr));

			if (dot == std::string::npos) break;
		}
	}

	gen_name(node->get_asname(), EC_STORE);
}

ASTNode* BaseCodeGenerator::visit_name(NameNode* node)
{
	set_lineno(node->get_line());
	gen_name(node->get_name(), node->get_context());
	return node;
}

ASTNode* BaseCodeGenerator::visit_keyword(KeywordNode* node)
{
	load_const(space->wrap_str(context, node->get_arg()));
	node->get_value()->visit(this);

	return node;
}

ASTNode* BaseCodeGenerator::visit_list(ListNode* node)
{
	set_lineno(node->get_line());
	std::vector<ASTNode*>& elements = node->get_elements();
	std::size_t eltcount = elements.size();

	ExprContext ctx = node->get_context();

	for (auto elt : elements) {
		elt->visit(this);
	}

	if (ctx == ExprContext::EC_LOAD) emit_op_arg(BUILD_LIST, eltcount);


	return node;
}

ASTNode* BaseCodeGenerator::visit_const(ConstNode* node)
{
	set_lineno(node->get_line());
	load_const(node->get_value());

	return node;
}

ASTNode* BaseCodeGenerator::visit_number(NumberNode* node)
{
	set_lineno(node->get_line());
	load_const(node->get_value());

	return node;
}

ASTNode* BaseCodeGenerator::visit_pass(PassNode* node)
{
	set_lineno(node->get_line());

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

ASTNode* BaseCodeGenerator::visit_set(SetNode* node)
{
	set_lineno(node->get_line());

	std::vector<ASTNode*>& elements = node->get_elements();
	std::size_t eltcount = elements.size();

	for (auto elt : elements) {
		elt->visit(this);
	}

	emit_op_arg(BUILD_SET, eltcount);

	return node;
}

ASTNode* BaseCodeGenerator::visit_subscript(SubscriptNode* node)
{
	set_lineno(node->get_line());

	if (node->get_context() != EC_AUGSTORE) {
		node->get_value()->visit(this);
	}

	ASTNode* slice = node->get_slice();
	ExprContext ctx = node->get_context();
	if (slice->get_tag() == NT_INDEX) {
		if (ctx != EC_AUGSTORE) {
			IndexNode* index = dynamic_cast<IndexNode*>(slice);
			index->get_value()->visit(this);
		}
	}

	emit_op(subscr_op[ctx]);

	return node;
}

ASTNode* BaseCodeGenerator::visit_try(TryNode* node)
{
	if (!(node->get_finalbody()))
		return visit_try_except(node);
	else
		return visit_try_finally(node);
}

ASTNode* BaseCodeGenerator::visit_try_except(TryNode* node)
{
	set_lineno(node->get_line());

	CodeBlock* body = new_block();
	CodeBlock* exc = new_block();
	CodeBlock* otherwise = new_block();
	CodeBlock* end = new_block();
	CodeBlock* next_exc;

	emit_jump(SETUP_EXCEPT, exc);
	push_frame_block(F_EXCEPT, body);
	visit_sequence(node->get_body());
	emit_op(POP_BLOCK);
	pop_frame_block();
	emit_jump(JUMP_FORWARD, otherwise);

	use_next_block(exc);
	std::vector<ExceptHandlerNode*>& handlers = node->get_handlers();
	for (auto handler : handlers) {
		set_lineno(handler->get_line());
		next_exc = new_block();
		ASTNode* type = handler->get_type();
		if (type) {
			emit_op(DUP_TOP);
			type->visit(this);
			emit_op_arg(COMPARE_OP, 10);
			emit_jump(JUMP_IF_FALSE_OR_POP, next_exc, true);
		}
		emit_op(POP_TOP);
		std::string& name = handler->get_name();
		if (name != "") {
			CodeBlock* cleanup_body, *cleanup_end;
			cleanup_end = new_block();
			gen_name(name, EC_STORE);
			emit_op(POP_TOP);
			/*
              try:
                  # body
              except type as name:
                  try:
                      # body
                  finally:
                      name = None
                      del name
            */

			emit_jump(SETUP_FINALLY, cleanup_end);
			cleanup_body = new_block();
			push_frame_block(F_FINALLY, cleanup_body);
			visit_sequence(handler->get_body());
			emit_op(POP_BLOCK);
			emit_op(POP_EXCEPT);
			pop_frame_block();

			load_const(space->wrap_None());
			use_next_block(cleanup_end);
			push_frame_block(F_FINALLY_END, cleanup_end);
			load_const(space->wrap_None());
			gen_name(name, EC_STORE);
			gen_name(name, EC_DEL);

			emit_op(END_FINALLY);
			pop_frame_block();
		} else {
			emit_op(POP_TOP);
			emit_op(POP_TOP);
			CodeBlock* cleanupbody = new_block();
			use_next_block(cleanupbody);
			push_frame_block(F_FINALLY, cleanupbody);
			visit_sequence(handler->get_body());
			emit_op(POP_EXCEPT);
			pop_frame_block();
		}
		emit_jump(JUMP_FORWARD, end);
		use_next_block(next_exc);
	}

	emit_op(END_FINALLY);
	use_next_block(otherwise);
	visit_sequence(node->get_orelse());
	use_next_block(end);

	return node;
}

ASTNode* BaseCodeGenerator::visit_try_finally(TryNode* node)
{
	set_lineno(node->get_line());
	CodeBlock* end = new_block();
	emit_jump(SETUP_FINALLY, end);
	CodeBlock* body = new_block();
	use_next_block(body);
	push_frame_block(F_FINALLY, body);
	if (node->get_handlers().size() > 0)
		visit_try_except(node);
	else
		visit_sequence(node->get_body());
	emit_op(POP_BLOCK);
	pop_frame_block();

	load_const(space->wrap_None());
	use_next_block(end);
	push_frame_block(F_FINALLY_END, end);
	visit_sequence(node->get_finalbody());
	emit_op(END_FINALLY);
	pop_frame_block();

	return node;
}

ASTNode* BaseCodeGenerator::visit_tuple(TupleNode* node)
{
	set_lineno(node->get_line());
	std::vector<ASTNode*>& elements = node->get_elements();
	std::size_t eltcount = elements.size();

	ExprContext ctx = node->get_context();
	if (ctx == ExprContext::EC_STORE) {
		for (std::size_t i = 0; i < eltcount; i++) {
			ASTNode* elt = elements[i];
			/* TODO: Starred assignment PEP 3132 */
		}
	}

	for (auto elt : elements) {
		elt->visit(this);
	}

	if (ctx == ExprContext::EC_LOAD) emit_op_arg(BUILD_TUPLE, eltcount);


	return node;
}

ASTNode* BaseCodeGenerator::visit_unaryop(UnaryOpNode* node)
{
	set_lineno(node->get_line());
	node->get_operand()->visit(this);
	emit_op(_unaryop(node->get_op()));

	return node;
}

ASTNode* BaseCodeGenerator::visit_while(WhileNode* node)
{
	set_lineno(node->get_line());

	ASTNode* test = node->get_test();
	int test_const = expr_constant(test);
	if (test_const == 0) {
		if (ASTNode* orelse = node->get_orelse())
			visit_sequence(orelse);
	} else {
		CodeBlock* end = new_block();
		CodeBlock* loop = new_block();
		CodeBlock* anchor = nullptr;

		if (test_const == -1) {
			anchor = new_block();
		}

		CodeBlock* orelse = nullptr;
		if (node->get_orelse()) orelse = new_block();

		emit_jump(SETUP_LOOP, end);
		use_next_block(loop);
		push_frame_block(FrameType::F_LOOP, loop);

		if (test_const == -1) {
			node->get_test()->visit(this);
			emit_jump(POP_JUMP_IF_FALSE, anchor, true);
		}

		visit_sequence(node->get_body());
		emit_jump(JUMP_ABSOLUTE, loop, true);

		if (test_const == -1) {
			use_next_block(anchor);
		}
		emit_op(POP_BLOCK);
		pop_frame_block();

		if (node->get_orelse()) visit_sequence(node->get_orelse());

		use_next_block(end);
	}

	return node;
}

ModuleCodeGenerator::ModuleCodeGenerator(mtpython::vm::ThreadContext* context, mtpython::tree::ASTNode* module, SymtableVisitor* symtab, CompileInfo* info) : BaseCodeGenerator("module", context, module, symtab, -1, info)
{
	compile(module);
}

void ModuleCodeGenerator::compile(ASTNode* module)
{
	module->visit(this);
}

FunctionCodeGenerator::FunctionCodeGenerator(const std::string& name, mtpython::vm::ThreadContext* context, mtpython::tree::ASTNode* tree, SymtableVisitor* symtab, int lineno, CompileInfo* info) : BaseCodeGenerator(name, context, tree, symtab, lineno, info)
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

ClassCodeGenerator::ClassCodeGenerator(const std::string& name, mtpython::vm::ThreadContext* context, mtpython::tree::ASTNode* tree, SymtableVisitor* symtab, int lineno, CompileInfo* info) : BaseCodeGenerator(name, context, tree, symtab, lineno, info)
{
	compile(tree);
}

void ClassCodeGenerator::compile(ASTNode* tree)
{
	ClassDefNode* classdef = dynamic_cast<ClassDefNode*>(tree);

	set_argcount(0);

	gen_name("__name__", ExprContext::EC_LOAD);
	gen_name("__module__", ExprContext::EC_STORE);

	ASTNode* body = classdef->get_body();
	if (body) visit_sequence(body);

	load_const(space->wrap_None());
	emit_op(RETURN_VALUE);
}
