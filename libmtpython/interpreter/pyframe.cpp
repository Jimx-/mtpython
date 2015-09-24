#include <exception>
#include <interpreter/pyframe.h>

#include "interpreter/pyframe.h"
#include "interpreter/function.h"
#include "tools/opcode.h"
#include "macros.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;
using namespace mtpython::vm;

static BreakUnwinder break_unwinder;

class ExitFrameException : public std::exception { };
class ReturnException : public ExitFrameException { };

void FrameBlock::cleanup(PyFrame* frame)
{
    frame->drop_values_until(level);
}

int LoopBlock::handle(PyFrame* frame, StackUnwinder* unwinder)
{
	if (unwinder->why() == WhyCode::WHY_BREAK) {
		cleanup(frame);
		return handler;
	}

	return handler;
}

int ExceptBlock::handle(PyFrame* frame, StackUnwinder* unwinder)
{
	ExceptionUnwinder* as_exc = dynamic_cast<ExceptionUnwinder*>(unwinder);
	cleanup(frame);

	FrameBlock* finally_block = new FinallyBlock(0, frame->value_stack_level());
	frame->push_block(finally_block);

	frame->push_value(frame->get_space()->wrap(as_exc));
	const InterpError& error = as_exc->get_error();
	frame->push_value(error.get_value());
	frame->push_value(error.get_type());

	return handler;
}

int FinallyBlock::handle(PyFrame* frame, StackUnwinder* unwinder)
{
	cleanup(frame);
	ExceptionUnwinder* as_exc = dynamic_cast<ExceptionUnwinder*>(unwinder);
	if (as_exc) {
		ObjSpace* space = frame->get_space();
		frame->push_value(space->wrap(as_exc));
	}

	return handler;
}

PyFrame::PyFrame(ThreadContext* context, Code* code, M_BaseObject* globals)
{
	this->context = context;
    space = context->get_space();
	
	this->pycode = dynamic_cast<PyCode*>(code);
	int nlocals = pycode->get_nlocals();
	local_vars.resize(nlocals, nullptr);

	this->globals = globals;
	this->locals = globals;

	pc = -1;
}

M_BaseObject* PyFrame::exec()
{
	return execute_frame();
}

M_BaseObject* PyFrame::execute_frame()
{
	M_BaseObject* retval;

	context->enter(this);

	int next_pc = pc + 1;

	try {
		retval = dispatch(context, pycode, next_pc);
	} catch (...) {
		throw;
	}

	context->leave(this);

	return retval;
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
	} catch (InterpError& e) {
		next_pc = handle_interp_error(e);
	}

	return next_pc;
}

int PyFrame::handle_interp_error(InterpError& exc)
{
	FrameBlock* block = unwind_stack(WhyCode::WHY_EXCEPTION);

	/* for debug propose */
	std::string info = space->unwrap_str(exc.get_value());

	if (!block) {	/* no handler */
		throw exc;
	}

	ExceptionUnwinder* unwinder = new ExceptionUnwinder(exc);
	int next_pc = block->handle(this, unwinder);

	return next_pc;
}

int PyFrame::dispatch_bytecode(ThreadContext* context, std::vector<unsigned char>& bytecode, int next_pc)
{
	while (true) {
		pc = next_pc;

		unsigned char opcode = bytecode[next_pc];
		next_pc++;

		int arg = 0;
		if (opcode >= HAVE_ARGUMENT) {
			unsigned char lo = bytecode[next_pc++];
			unsigned char hi = bytecode[next_pc++];

			arg = (hi << 8) | lo;
		}

		while (opcode == EXTENDED_ARG) {
			opcode = bytecode[next_pc++];
			unsigned char lo = bytecode[next_pc++];
			unsigned char hi = bytecode[next_pc++];

			arg = (arg << 16) | (hi << 8) | lo;
		}

		if (opcode == RETURN_VALUE) {
			throw ReturnException();
		}

		if (opcode == END_FINALLY) {
			StackUnwinder* unwinder = dynamic_cast<StackUnwinder*>(end_finally());
			if (unwinder) {
				FrameBlock* block = unwind_stack(unwinder->why());
				if (!block) {
					push_value(unwinder->unhandle());
					throw ReturnException();
				} else {
					next_pc = block->handle(this, unwinder);
				}
			}

			return next_pc;
		}

		if (opcode == JUMP_ABSOLUTE)
			return jump_absolute(arg);
		else if (opcode == POP_TOP)
			pop_top(arg, next_pc);
		else if (opcode == BINARY_ADD)
			binary_add(arg, next_pc);
		else if (opcode == BINARY_SUBTRACT)
			binary_sub(arg, next_pc);
		else if (opcode == BINARY_MULTIPLY)
			binary_mul(arg, next_pc);
		else if (opcode == LOAD_CONST)
			load_const(arg, next_pc);
		else if (opcode == LOAD_FAST)
			load_fast(arg, next_pc);
		else if (opcode == STORE_FAST)
			store_fast(arg, next_pc);
		else if (opcode == JUMP_FORWARD)
			next_pc = jump_forward(arg, next_pc);
		else if (opcode == POP_JUMP_IF_FALSE)
			next_pc = pop_jump_if_false(arg, next_pc);
		else if (opcode == LOAD_GLOBAL)
			load_global(arg, next_pc);
		else if (opcode == CALL_FUNCTION)
			call_function(arg, next_pc);
		else if (opcode == MAKE_FUNCTION)
			make_function(arg, next_pc);
		else if (opcode == DUP_TOP)
			dup_top(arg, next_pc);
		else if (opcode == ROT_THREE)
			rot_three(arg, next_pc);
		else if (opcode == ROT_TWO)
			rot_two(arg, next_pc);
		else if (opcode == COMPARE_OP)
			compare_op(arg, next_pc);
		else if (opcode == JUMP_IF_FALSE_OR_POP)
			next_pc = jump_if_false_or_pop(arg, next_pc);
		else if (opcode == BUILD_TUPLE)
			build_tuple(arg, next_pc);
        else if (opcode == SETUP_LOOP)
            setup_loop(arg, next_pc);
        else if (opcode == GET_ITER)
            get_iter(arg, next_pc);
        else if (opcode == FOR_ITER)
            next_pc = for_iter(arg, next_pc);
        else if (opcode == POP_BLOCK)
            _pop_block(arg, next_pc);
		else if (opcode == BREAK_LOOP)
			next_pc = break_loop(arg, next_pc);
		else if (opcode == UNARY_POSITIVE)
			unary_positive(arg, next_pc);
		else if (opcode == UNARY_NEGATIVE)
			unary_negative(arg, next_pc);
		else if (opcode == UNARY_NOT)
			unary_not(arg, next_pc);
		else if (opcode == UNARY_INVERT)
			unary_invert(arg, next_pc);
		else if (opcode == SETUP_FINALLY)
			setup_finally(arg, next_pc);
		else if (opcode == SETUP_EXCEPT)
			setup_except(arg, next_pc);
		else if (opcode == POP_EXCEPT)
			pop_except(arg, next_pc);
		else if (opcode == DELETE_FAST)
			delete_fast(arg, next_pc);
		else if (opcode == LOAD_ATTR)
			load_attr(arg, next_pc);
		else if (opcode == STORE_ATTR)
			store_attr(arg, next_pc);
		else if (opcode == DELETE_ATTR)
			delete_attr(arg, next_pc);
		else if (opcode == IMPORT_NAME)
			import_name(arg, next_pc);
		else if (opcode == STORE_GLOBAL)
			store_global(arg, next_pc);
		else if (opcode == LOAD_NAME)
			load_name(arg, next_pc);
		else if (opcode == STORE_NAME)
			store_name(arg, next_pc);
		else if (opcode == BINARY_SUBSCR)
			binary_subscr(arg, next_pc);
	}
}

FrameBlock* PyFrame::unwind_stack(int why)
{
	while (block_stack.size() > 0) {
		FrameBlock* block = pop_block();
		if (block->handling_mask() & why) {
			return block;
		}

		block->cleanup(this);
	}

	return nullptr;
}

int PyFrame::unwind_stack_jump(StackUnwinder* unwinder)
{
	FrameBlock* block = unwind_stack(unwinder->why());
	if (!block) {
		throw mtpython::BytecodeCorruption("Stack unwind error");
	}

	return block->handle(this, unwinder);
}

M_BaseObject* PyFrame::get_const(int index)
{
	return pycode->get_consts()[index];
}

M_BaseObject* PyFrame::get_name(int index)
{
	return pycode->get_names()[index];
}

const std::string& PyFrame::get_localname(int index)
{
	return pycode->get_varnames()[index];
}

#define DEF_BINARY_OPER(name) \
	void PyFrame::binary_##name(int arg, int next_pc) \
	{ \
		M_BaseObject* obj2 = pop_value_untrack(); \
		M_BaseObject* obj1 = pop_value_untrack(); \
		M_BaseObject* result = space->name(obj1, obj2); \
		push_value(result);	\
		context->gc_track_object(obj1);	\
		context->gc_track_object(obj2);	\
	}

#define DEF_BINARY_OPER_ALIAS(name, opname) \
	void PyFrame::binary_##name(int arg, int next_pc) \
	{ \
		M_BaseObject* obj2 = pop_value_untrack(); \
		M_BaseObject* obj1 = pop_value_untrack(); \
		M_BaseObject* result = space->opname(obj1, obj2); \
		push_value(result);	\
		context->gc_track_object(obj1);	\
		context->gc_track_object(obj2);	\
	}

DEF_BINARY_OPER(add)
DEF_BINARY_OPER(sub)
DEF_BINARY_OPER(mul)
DEF_BINARY_OPER_ALIAS(subscr, getitem)

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
	if (!value) {
		throw InterpError::format(space, space->UnboundLocalError_type(), "local variable '%s' referenced before assignment", get_localname(arg).c_str());
	}
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
	std::string unwrapped_name = space->unwrap_str(name);

	M_BaseObject* value = space->finditem_str(globals, unwrapped_name);
	if (!value) {
		value = space->get_builtin()->get_dict_value(space, unwrapped_name);
		if (!value) throw InterpError::format(space, space->NameError_type(), "global name '%s' not found", unwrapped_name.c_str());
	}

	push_value(value);
}

void PyFrame::call_function_common(int arg, M_BaseObject* star, M_BaseObject* starstar)
{
	int nargs = arg & 0xff;
	int nkwargs = (arg >> 8) & 0xff;

	std::vector<M_BaseObject*> args;

	std::vector<std::string> keywords;
	std::vector<M_BaseObject*> keyword_values;

	if (nkwargs > 0) {
		keywords.resize(nkwargs);
		keyword_values.resize(nkwargs);
		while (true) {
			nkwargs--;
			if (nkwargs < 0) break;

			M_BaseObject* value = pop_value_untrack();
			M_BaseObject* key = pop_value_untrack();

			keywords[nkwargs] = space->unwrap_str(key);
			keyword_values[nkwargs] = value;
		}
	}

	pop_values_untrack(nargs, args);

	Arguments arguments(space, args, keywords, keyword_values);
	M_BaseObject* func = pop_value_untrack();
	M_BaseObject* result = space->call_args(context, func, arguments);
	push_value(result);
}

void PyFrame::call_function(int arg, int next_pc)
{
	call_function_common(arg);
}

void PyFrame::make_function(int arg, int next_pc)
{
	M_BaseObject* qualname = pop_value_untrack();
	M_BaseObject* code_obj = pop_value_untrack();

	PyCode* code = dynamic_cast<PyCode*>(code_obj);
	if (!code) throw InterpError(space->TypeError_type(), space->wrap_str("expected code object"));

	M_BaseObject* func = new Function(space, code, globals);
	push_value(space->wrap(func));
}

int PyFrame::jump_absolute(int arg)
{
	return arg;
}

int PyFrame::jump_forward(int arg, int next_pc)
{
	next_pc += arg;
	return next_pc;
}

int PyFrame::pop_jump_if_false(int arg, int next_pc)
{
	M_BaseObject* value = pop_value_untrack();
	if (!space->is_true(value)) {
		next_pc = arg;
	}

	context->gc_track_object(value);
	return next_pc;
}

void PyFrame::dup_top(int arg, int next_pc)
{
	push_value(peek_value());
}

void PyFrame::rot_three(int arg, int next_pc)
{
	M_BaseObject* v1 = pop_value_untrack();
	M_BaseObject* v2 = pop_value_untrack();
	M_BaseObject* v3 = pop_value_untrack();
	push_value(v1);
	push_value(v3);
	push_value(v2);
}

void PyFrame::compare_op(int arg, int next_pc)
{
	M_BaseObject* v2 = pop_value_untrack();
	M_BaseObject* v1 = pop_value_untrack();

	M_BaseObject* result;
	switch (arg) {
	case 0:
		result = space->lt(v1, v2);
		break;
	case 1:
		result = space->le(v1, v2);
		break;
	case 2:
		result = space->eq(v1, v2);
		break;
	case 3:
		result = space->ne(v1, v2);
		break;
	case 4:
		result = space->gt(v1, v2);
		break;
	case 5:
		result = space->ge(v1, v2);
		break;
	}

	push_value(result);
	context->gc_track_object(v1);
	context->gc_track_object(v2);
}

int PyFrame::jump_if_false_or_pop(int arg, int next_pc)
{
	M_BaseObject* value = peek_value();
	int result;
	if (!space->is_true(value))
		result = arg;
	else {
		pop_value();
		result = next_pc;
	}


	return result;
}

void PyFrame::rot_two(int arg, int next_pc)
{
	M_BaseObject* v1 = pop_value_untrack();
	M_BaseObject* v2 = pop_value_untrack();
	push_value(v1);
	push_value(v2);
}

void PyFrame::build_tuple(int arg, int next_pc)
{
	std::vector<M_BaseObject*> args;
	pop_values_untrack(arg, args);
	M_BaseObject* tup = space->new_tuple(args);
	push_value(tup);
}

void PyFrame::setup_loop(int arg, int next_pc)
{
	FrameBlock* loop_block = new LoopBlock(next_pc + arg, value_stack.size());
	push_block(loop_block);
}

void PyFrame::get_iter(int arg, int next_pc)
{
    M_BaseObject* iterable = pop_value_untrack();
    M_BaseObject* iterator = space->iter(iterable);
    push_value(iterator);
    context->gc_track_object(iterable);
}

int PyFrame::for_iter(int arg, int next_pc)
{
    M_BaseObject* iterator = peek_value();
    M_BaseObject* next_obj = nullptr;
    try {
        next_obj = space->next(iterator);
        push_value(next_obj);
    } catch (InterpError& e) {
        if (!e.match(space, space->StopIteration_type())) {
            throw e;
        }

        pop_value();
        next_pc += arg;
    }

    return next_pc;
}

void PyFrame::_pop_block(int arg, int next_pc)
{
    FrameBlock* block = pop_block();
    block->cleanup(this);
    SAFE_DELETE(block);
}

int PyFrame::break_loop(int arg, int next_pc)
{
	return unwind_stack_jump(&break_unwinder);
}

#define DEF_UNARY_OPER(opname, name) \
	void PyFrame::unary_##opname(int arg, int next_pc) \
	{ \
		M_BaseObject* obj = pop_value_untrack(); \
		M_BaseObject* result = space->name(obj); \
		push_value(result);	\
		context->gc_track_object(obj);	\
	}

DEF_UNARY_OPER(positive, pos)
DEF_UNARY_OPER(negative, neg)
DEF_UNARY_OPER(invert, invert)
DEF_UNARY_OPER(not, not_)

void PyFrame::setup_finally(int arg, int next_pc)
{
	FrameBlock* finally_block = new FinallyBlock(next_pc + arg, value_stack.size());
	push_block(finally_block);
}

void PyFrame::setup_except(int arg, int next_pc)
{
	FrameBlock* except_block = new ExceptBlock(next_pc + arg, value_stack.size());
	push_block(except_block);
}

void PyFrame::pop_except(int arg, int next_pc)
{
	FrameBlock* block = pop_block();
	block->cleanup(this);
	SAFE_DELETE(block);
}

void PyFrame::delete_fast(int arg, int next_pc)
{
	M_BaseObject* value = local_vars[arg];
	if (!value) {
		throw InterpError::format(space, space->UnboundLocalError_type(), "local variable '%s' referenced before assignment", get_localname(arg).c_str());
	}
	local_vars[arg] = nullptr;
}

M_BaseObject* PyFrame::end_finally()
{
	M_BaseObject* top = pop_value_untrack();
	if (space->i_is(top, space->wrap_None())) return nullptr;

	/* stack : [unwinder] 			case of a finally */
	StackUnwinder* as_unwinder = dynamic_cast<StackUnwinder*>(top);
	if (as_unwinder) return top;

	/* stack : [ExceptionUnwinder, value, type]		case of an except */
	pop_value();		/* pop value */
	return pop_value_untrack();
}

void PyFrame::load_attr(int arg, int next_pc)
{
	M_BaseObject* obj = pop_value_untrack();
	M_BaseObject* attr = get_name(arg);
	M_BaseObject* value = space->getattr(obj, attr);
	push_value(value);
	context->gc_track_object(obj);
}
void PyFrame::store_attr(int arg, int next_pc)
{
	M_BaseObject* obj = pop_value_untrack();
	M_BaseObject* attr = get_name(arg);
	M_BaseObject* value = pop_value_untrack();
	space->setattr(obj, attr, value);
	context->gc_track_object(obj);
	context->gc_track_object(value);
}

void PyFrame::delete_attr(int arg, int next_pc)
{
	M_BaseObject* obj = pop_value_untrack();
	M_BaseObject* attr = get_name(arg);
	space->delattr(obj, attr);
	context->gc_track_object(obj);
}

void PyFrame::import_name(int arg, int next_pc)
{
	M_BaseObject* mod_name = get_name(arg);
	M_BaseObject* from_list = pop_value_untrack();
	M_BaseObject* level = pop_value_untrack();

	M_BaseObject* import_func = space->get_builtin()->get_dict_value(space, "__import__");
	if (!import_func) {
		throw InterpError(space->ImportError_type(), space->wrap_str("__import__ not found"));
	}

	M_BaseObject* wrapped_locals = locals;
	if (!wrapped_locals) wrapped_locals = space->wrap_None();

	M_BaseObject* wrapped_globals = globals;
	M_BaseObject* obj = space->call_function(context, import_func, {mod_name, wrapped_globals, wrapped_locals, from_list, level});

	push_value(obj);
	context->gc_track_object(mod_name);
	context->gc_track_object(from_list);
	context->gc_track_object(level);
}

void PyFrame::store_global(int arg, int next_pc)
{
	M_BaseObject* w_name = get_name(arg);
	std::string name = space->unwrap_str(w_name);
	M_BaseObject* value = pop_value_untrack();
	space->setitem_str(globals, name, value);
	context->gc_track_object(value);
}

void PyFrame::load_name(int arg, int next_pc)
{
	M_BaseObject* w_name = get_name(arg);
	std::string name = space->unwrap_str(w_name);
	M_BaseObject* value = nullptr;

	if (locals != globals) {
		value = space->finditem_str(locals, name);
		if (value) {
			push_value(value);
			return;
		}
	}

	value = space->finditem_str(globals, name);
	if (!value) {
		value = space->get_builtin()->get_dict_value(space, name);
		if (!value) throw InterpError::format(space, space->NameError_type(), "name '%s' not found", name.c_str());
	}

	push_value(value);
}

void PyFrame::store_name(int arg, int next_pc)
{
	M_BaseObject* w_name = get_name(arg);
	std::string name = space->unwrap_str(w_name);
	M_BaseObject* value = pop_value_untrack();
	space->setitem_str(locals, name, value);
	context->gc_track_object(value);
}

