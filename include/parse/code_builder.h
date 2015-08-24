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

class Instruction {
private:
	unsigned char op;
	int arg;
	int lineno;
public:
	Instruction(unsigned char op, int arg=0) { this->op = op; this->arg = arg; this->lineno = 0; }
	void set_lineno(int lineno) { this->lineno = lineno; }

	int size() { if (op >= HAVE_ARGUMENT) { if (arg > 0xffff) return 6; else return 3; } return 1; }
	unsigned char get_op() { return op; }
	int get_arg() { return arg; }
	int get_lineno() { return lineno; }
};

class CodeBlock {
private:
	std::vector<Instruction*> instructions;
	CodeBlock* next;
	int depth;
	int offset;

	bool _have_return;
public:
	CodeBlock() { _have_return = false; next = nullptr; }
	
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

protected:
	mtpython::objects::ObjSpace* space;
	CompileInfo* compile_info;
	
	std::unordered_map<std::string, int> names;
	std::unordered_map<std::string, int> varnames;
	std::unordered_map<std::string, int> freevars;
	std::unordered_map<std::string, int> cellvars;

	int add_name(std::unordered_map<std::string, int>& container, std::string& id);

	void emit_op(unsigned char op);
	void emit_op_arg(unsigned char op, int arg);

	int opcode_stack_effect(unsigned char op, int arg);
public:
	CodeBuilder(std::string& name, mtpython::objects::ObjSpace* space, Scope* scope, int first_lineno, CompileInfo* info);

	mtpython::interpreter::PyCode* build();

	void set_block(CodeBlock* block) { current_block = block; }
	void set_lineno(int lineno);

	void load_const(mtpython::objects::M_BaseObject* obj);
};

}
}

#endif /* _CODE_BUILDER_H_ */
