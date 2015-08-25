#ifndef _CODE_BUILDER_H_
#define _CODE_BUILDER_H_

#include "tree/nodes.h"
#include "tree/visitors/generic_visitor.h"
#include "objects/obj_space.h"
#include "tools/opcode.h"
#include "parse/symtable.h"
#include "interpreter/pycode.h"
#include "parse/compile_info.h"
#include <vector>
#include <unordered_map>

namespace mtpython {
namespace parse {

class CodeBlock;

class Instruction {
private:
	unsigned char op;
	int arg;
	int lineno;

	CodeBlock* jump;
	bool absolute_jump;
public:
	Instruction(unsigned char op, int arg=0) { this->op = op; this->arg = arg; this->lineno = 0; jump = nullptr; }
	void set_lineno(int lineno) { this->lineno = lineno; }

	int size() { if (op >= HAVE_ARGUMENT) { if (arg > 0xffff) return 6; else return 3; } return 1; }
	unsigned char get_op() { return op; }
	int get_arg() { return arg; }
	void set_arg(int arg) { this->arg = arg; }
	int get_lineno() { return lineno; }

	void jump_to(CodeBlock* target, bool absolute=false) { jump = target; absolute_jump = absolute; }
	bool has_jump() { return (jump != nullptr); }
	bool is_absolute() { return absolute_jump; }
	CodeBlock* get_target() { return jump; }
};

class CodeBlock {
private:
	std::vector<Instruction*> instructions;
	CodeBlock* next;
	int depth;
	int offset;
	bool seen;

	bool _have_return;

	void dfs(std::vector<CodeBlock*>& blocks);
public:
	CodeBlock() { _have_return = false; next = nullptr; seen = false; }
	
	std::vector<Instruction*>& get_instructions() { return instructions; }
	void append_instruction(Instruction* inst) { instructions.push_back(inst); }

	int code_size() { int sum = 0; for (auto inst : instructions) { sum += inst->size(); } return sum; }

	void get_code(std::vector<unsigned char>& code);

	int get_depth() { return depth; }
	void set_depth(int depth) { this->depth = depth; }
	int get_offset() { return offset; }
	void set_offset(int offset) { this->offset = offset; }

	bool have_return() { return _have_return; }
	void set_have_return(bool v) { _have_return = v; }

	CodeBlock* get_next() { return next; }
	void set_next(CodeBlock* block) { next = block; }

	void get_block_list(std::vector<CodeBlock*>& blocks);
};

class CodeBuilder : public mtpython::tree::GenericVisitor {
private:
	std::string name;
	int first_lineno;
	int lineno;
	bool lineno_set;

	int argcount;
	int kwonlyargcount;

	CodeBlock* first_block;
	CodeBlock* current_block;

	/* const -> index mapping */
	std::unordered_map<mtpython::objects::M_BaseObject*, int> consts;

	virtual int get_code_flags() { return 0; }
	
	void append_instruction(Instruction* inst);

	int add_const(mtpython::objects::M_BaseObject* obj);
	int get_stacksize(std::vector<CodeBlock*>& blocks);
	void build_lnotab(std::vector<CodeBlock*>& blocks, std::vector<unsigned char>& lnotab);
	void patch_jump(std::vector<CodeBlock*>& blocks);

protected:
	mtpython::objects::ObjSpace* space;
	CompileInfo* compile_info;
	
	bool auto_add_return_value;
	
	std::unordered_map<std::string, int> names;
	std::unordered_map<std::string, int> varnames;
	std::unordered_map<std::string, int> freevars;
	std::unordered_map<std::string, int> cellvars;

	int add_name(std::unordered_map<std::string, int>& container, std::string& id);

	Instruction* emit_op(unsigned char op);
	Instruction* emit_op_arg(unsigned char op, int arg);
	void emit_jump(unsigned char op, CodeBlock* target, bool absolute=false);

	int opcode_stack_effect(unsigned char op, int arg);

	CodeBlock* new_block() { return new CodeBlock(); }
	void use_block(CodeBlock* block) { current_block = block; }
	CodeBlock* use_next_block(CodeBlock* block=nullptr);
	void set_lineno(int lineno);

	int expr_constant(mtpython::tree::ASTNode* node);
	
	void load_const(mtpython::objects::M_BaseObject* obj);
public:
	CodeBuilder(std::string& name, mtpython::objects::ObjSpace* space, Scope* scope, int first_lineno, CompileInfo* info);

	mtpython::interpreter::PyCode* build();
};

}
}

#endif /* _CODE_BUILDER_H_ */
