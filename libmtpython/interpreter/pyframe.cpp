#include "interpreter/pyframe.h"
#include "interpreter/pycode.h"
#include "tools/opcode.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;
using namespace mtpython::vm;

PyFrame::PyFrame(ThreadContext* context, Code* code, M_BaseObject* globals)
{
	this->context = context;
	this->pycode = dynamic_cast<PyCode*>(code);
	this->globals = globals;

	pc = -1;
}

M_BaseObject* PyFrame::exec()
{
	return execute_frame();
}

M_BaseObject* PyFrame::execute_frame()
{
	M_BaseObject* retval;

	try {
		int next_pc = pc + 1;

		try {
			retval = dispatch(context, pycode, next_pc);
		} 
		catch (...) {
			throw;
		}
	}
	catch (...) {
		throw;
	}

	return nullptr;
}

M_BaseObject* PyFrame::dispatch(ThreadContext* context, Code* code, int next_pc)
{
	PyCode* pycode = dynamic_cast<PyCode*>(code);
	if (!pycode) return nullptr;

	std::vector<char>& bytecode = pycode->get_code();

	try {
		while (true) {
			next_pc = execute_bytecode(context, bytecode, next_pc);
		}
	}
	catch (...) {
		throw;
	}
}

int PyFrame::execute_bytecode(ThreadContext* context, std::vector<char>& bytecode, int next_pc)
{
	try {
		next_pc = dispatch_bytecode(context, bytecode, next_pc);
	}
	catch (...) {
		throw;
	}

	return next_pc;
}

int PyFrame::dispatch_bytecode(ThreadContext* context, std::vector<char>& bytecode, int next_pc)
{
	while (true) {
		pc = next_pc;

		char opcode = bytecode[next_pc];
		next_pc++;

		int arg = 0;
		if (opcode >= HAVE_ARGUMENT) {
			char lo = bytecode[next_pc++];
			char hi = bytecode[next_pc++];

			arg = (hi << 8) | lo;
		}

		while (opcode == EXTENDED_ARG) {
			opcode = bytecode[next_pc++];
			char lo = bytecode[next_pc++];
			char hi = bytecode[next_pc++];

			arg = (arg << 16) | (hi << 8) | lo;
		}

		if (opcode == BINARY_ADD)
			binary_add(arg, next_pc);
		if (opcode == LOAD_CONST)
			load_const(arg, next_pc);
	}
}

M_BaseObject* PyFrame::get_const(int index)
{
	return pycode->get_consts()[index];
}

#define DEF_BINARY_OPER(name) \
	void PyFrame::binary_##name(int arg, int next_pc) \
	{ \
		M_BaseObject* obj2 = pop_value_untrack(); \
		M_BaseObject* obj1 = pop_value_untrack(); \
		M_BaseObject* result = context->get_space()->##name(context, obj1, obj2); \
		push_value(result);	\
		context->gc_track_object(obj1);	\
		context->gc_track_object(obj2);	\
	}

DEF_BINARY_OPER(add)

void PyFrame::load_const(int arg, int next_pc)
{
	push_value(get_const(arg));
}