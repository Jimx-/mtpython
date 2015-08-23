#include "interpreter/pyframe.h"
#include "interpreter/pycode.h"
#include "tools/opcode.h"

#include <exception>

using namespace mtpython::interpreter;
using namespace mtpython::objects;
using namespace mtpython::vm;

class ExitFrameException : public std::exception { };
class ReturnException : public ExitFrameException { };

PyFrame::PyFrame(ThreadContext* context, Code* code, M_BaseObject* globals)
{
	this->context = context;
	
	this->pycode = dynamic_cast<PyCode*>(code);
	int nlocals = pycode->get_nlocals();
	local_vars.resize(nlocals, nullptr);

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

	std::vector<unsigned char>& bytecode = pycode->get_code();

	try {
		while (true) {
			next_pc = execute_bytecode(context, bytecode, next_pc);
		}
	}
	catch (ExitFrameException) {
		return pop_value_untrack();
	}
}

int PyFrame::execute_bytecode(ThreadContext* context, std::vector<unsigned char>& bytecode, int next_pc)
{
	try {
		next_pc = dispatch_bytecode(context, bytecode, next_pc);
	}
	catch (InterpError& e) {
		next_pc = handle_interp_error(e);
	}

	return next_pc;
}

int PyFrame::handle_interp_error(InterpError& exc)
{
	context->gc_track_object(exc.get_value());
	return 0;
}

int PyFrame::dispatch_bytecode(ThreadContext* context, std::vector<unsigned char>& bytecode, int next_pc)
{
	while (true) {
		pc = next_pc;

		unsigned char opcode = bytecode[next_pc];
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

		if (opcode == RETURN_VALUE) {
			throw ReturnException();
		}

		if (opcode == POP_TOP)
			pop_top(arg, next_pc);
		else if (opcode == BINARY_ADD)
			binary_add(arg, next_pc);
		else if (opcode == LOAD_CONST)
			load_const(arg, next_pc);
		else if (opcode == LOAD_FAST)
			load_fast(arg, next_pc);
		else if (opcode == STORE_FAST)
			store_fast(arg, next_pc);
		else if (opcode == LOAD_GLOBAL)
			load_global(arg, next_pc);
		else if (opcode == CALL_FUNCTION)
			call_function(arg, next_pc);
	}
}

M_BaseObject* PyFrame::get_const(int index)
{
	return pycode->get_consts()[index];
}

M_BaseObject* PyFrame::get_name(int index)
{
	return pycode->get_names()[index];
}

#define DEF_BINARY_OPER(name) \
	void PyFrame::binary_##name(int arg, int next_pc) \
	{ \
		M_BaseObject* obj2 = pop_value_untrack(); \
		M_BaseObject* obj1 = pop_value_untrack(); \
		M_BaseObject* result = context->get_space()->##name(obj1, obj2); \
		push_value(result);	\
		context->gc_track_object(obj1);	\
		context->gc_track_object(obj2);	\
	}

DEF_BINARY_OPER(add)

void PyFrame::pop_top(int arg, int next_pc)
{
	pop_value();
}

void PyFrame::load_const(int arg, int next_pc)
{
	push_value(get_const(arg));
}

void PyFrame::load_fast(int arg, int next_pc)
{
	M_BaseObject* value = local_vars[arg];
	push_value(value);
}

void PyFrame::store_fast(int arg, int next_pc)
{
	M_BaseObject* value = pop_value_untrack();
	local_vars[arg] = value;
	context->gc_track_object(value);
}

void PyFrame::load_global(int arg, int next_pc)
{
	M_BaseObject* name = get_name(arg);
	ObjSpace* space = context->get_space();
	std::string unwrapped_name = space->unwrap_str(name);

	M_BaseObject* value = space->getitem(globals, name);
	if (!value) {
		value = space->get_builtin()->get_dict_value(space, unwrapped_name);
		if (!value) throw InterpError::format(space, space->TypeError_type(), "global name %s not found", unwrapped_name.c_str());
	}

	push_value(value);
}

void PyFrame::call_function_common(int arg, M_BaseObject* star, M_BaseObject* starstar)
{
	ObjSpace* space = context->get_space();
	int nargs = arg & 0xff;
	int nkwargs = (arg >> 8) & 0xff;

	std::vector<M_BaseObject*> args;
	pop_values_untrack(arg, args);

	std::vector<M_BaseObject*> keywords;
	std::vector<M_BaseObject*> keyword_values;

	Arguments arguments(space, args);

	M_BaseObject* func = pop_value_untrack();
	
	M_BaseObject* result = space->call_args(context, func, arguments);
	push_value(result);
}

void PyFrame::call_function(int arg, int next_pc)
{
	call_function_common(arg);
}
