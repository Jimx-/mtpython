#include <exception>
#include <interpreter/pyframe.h>

#include "interpreter/pyframe.h"
#include "interpreter/function.h"
#include "interpreter/generator.h"
#include "tools/opcode.h"
#include "macros.h"
#include "consts.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;
using namespace mtpython::vm;

static BreakUnwinder break_unwinder;

class ExitFrameException : public std::exception {};
class ReturnException : public ExitFrameException {};
class YieldException : public ExitFrameException {};

void FrameBlock::cleanup(PyFrame* frame) { frame->drop_values_until(level); }

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

    FrameBlock* except_handler_block =
        new ExceptHandlerBlock(0, frame->value_stack_level());
    frame->push_block(except_handler_block);

    frame->push_value(frame->get_space()->wrap(frame->get_context(), as_exc));
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
        frame->push_value(space->wrap(frame->get_context(), as_exc));
    }

    return handler;
}

PyFrame::PyFrame(ThreadContext* context, Code* code, M_BaseObject* globals,
                 M_BaseObject* outer)
{
    this->context = context;
    space = context->get_space();

    this->pycode = dynamic_cast<PyCode*>(code);
    int nlocals = pycode->get_nlocals();
    local_vars.resize(nlocals, nullptr);

    this->globals = globals;
    this->locals = globals;

    _finished_execution = false;
    pc = -1;
    init_cells(outer, pycode);
}

void PyFrame::init_cells(M_BaseObject* outer, PyCode* code)
{
    int ncellvars = code->get_ncellvars();
    int nfreevars = code->get_nfreevars();

    if (!ncellvars && !nfreevars) return;

    if (!outer) {
        throw InterpError(space->TypeError_type(),
                          space->wrap_str(context, ""));
    }

    Function* outer_func = static_cast<Function*>(outer);
    const std::vector<M_BaseObject*>& closure = outer_func->get_closure();

    int closure_size = closure.size();
    if (closure_size != nfreevars) {
        throw InterpError(
            space->ValueError_type(),
            space->wrap_str(context,
                            "code object received a closure with a wrong"
                            " number of free variables"));
    }

    cells.resize(ncellvars + nfreevars);
    for (int i = 0; i < ncellvars; i++) {
        cells[i] = new (context) Cell();
    }
    for (int i = 0; i < nfreevars; i++) {
        cells[i + ncellvars] = static_cast<Cell*>(closure[i]);
    }
}

void PyFrame::mark_children(gc::GarbageCollector* gc)
{
    gc->mark_object(pycode);
    gc->mark_object(globals);
    gc->mark_object(locals);

    for (const auto& val : value_stack)
        gc->mark_object(val);

    for (const auto& obj : local_vars)
        gc->mark_object(obj);

    for (const auto& obj : cells)
        gc->mark_object(obj);
}

M_BaseObject* PyFrame::exec()
{
    if (pycode->get_flags() & CO_GENERATOR) {
        return space->wrap(context, new (context) GeneratorIterator(this));
    }

    return execute_frame();
}

M_BaseObject* PyFrame::execute_frame(M_BaseObject* arg)
{
    M_BaseObject* retval;

    context->enter(this);

    int next_pc = pc + 1;
    if (next_pc != 0) {
        push_value(arg);
    }

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
    } catch (YieldException) {
        return pop_value();
    } catch (ExitFrameException) {
        return pop_value();
    }
}

int PyFrame::execute_bytecode(ThreadContext* context,
                              std::vector<unsigned char>& bytecode, int next_pc)
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

    /* for debug purpose */
    std::string info = space->unwrap_str(exc.get_value());

    if (!block) { /* no handler */
        throw exc;
    }

    ExceptionUnwinder* unwinder = new (context) ExceptionUnwinder(exc);
    int next_pc = block->handle(this, unwinder);

    return next_pc;
}

int PyFrame::dispatch_bytecode(ThreadContext* context,
                               std::vector<unsigned char>& bytecode,
                               int next_pc)
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
            M_BaseObject* result = pop_value();
            FrameBlock* block = unwind_stack(WHY_RETURN);
            if (!block) {
                push_value(result);
                throw ReturnException();
            } else {
                StackUnwinder* unwinder = new (context) ReturnUnwinder(result);
                next_pc = block->handle(this, unwinder);
            }

            return next_pc;
        }

        if (opcode == END_FINALLY) {
            StackUnwinder* unwinder =
                dynamic_cast<StackUnwinder*>(end_finally());
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

        switch (opcode) {
        case JUMP_ABSOLUTE:
            return jump_absolute(arg);
        case POP_TOP:
            pop_top(arg, next_pc);
            break;
        case BINARY_ADD:
            binary_add(arg, next_pc);
            break;
        case BINARY_SUBTRACT:
            binary_sub(arg, next_pc);
            break;
        case BINARY_MULTIPLY:
            binary_mul(arg, next_pc);
            break;
        case BINARY_TRUE_DIVIDE:
            binary_truediv(arg, next_pc);
            break;
        case BINARY_FLOOR_DIVIDE:
            binary_floordiv(arg, next_pc);
            break;
        case BINARY_MODULO:
            binary_mod(arg, next_pc);
            break;
        case BINARY_LSHIFT:
            binary_lshift(arg, next_pc);
            break;
        case BINARY_RSHIFT:
            binary_rshift(arg, next_pc);
            break;
        case BINARY_AND:
            binary_and_(arg, next_pc);
            break;
        case BINARY_OR:
            binary_or_(arg, next_pc);
            break;
        case BINARY_XOR:
            binary_xor_(arg, next_pc);
            break;
        case BINARY_POWER:
            binary_pow(arg, next_pc);
            break;
        case INPLACE_ADD:
            binary_inplace_add(arg, next_pc);
            break;
        case INPLACE_SUBTRACT:
            binary_inplace_sub(arg, next_pc);
            break;
        case INPLACE_MULTIPLY:
            binary_inplace_mul(arg, next_pc);
            break;
        case INPLACE_TRUE_DIVIDE:
            binary_inplace_truediv(arg, next_pc);
            break;
        case INPLACE_FLOOR_DIVIDE:
            binary_inplace_floordiv(arg, next_pc);
            break;
        case INPLACE_MODULO:
            binary_inplace_mod(arg, next_pc);
            break;
        case INPLACE_LSHIFT:
            binary_inplace_lshift(arg, next_pc);
            break;
        case INPLACE_RSHIFT:
            binary_inplace_rshift(arg, next_pc);
            break;
        case INPLACE_AND:
            binary_inplace_and(arg, next_pc);
            break;
        case INPLACE_OR:
            binary_inplace_or(arg, next_pc);
            break;
        case INPLACE_XOR:
            binary_inplace_xor(arg, next_pc);
            break;
        case INPLACE_POWER:
            binary_inplace_pow(arg, next_pc);
            break;
        case LOAD_CONST:
            load_const(arg, next_pc);
            break;
        case LOAD_FAST:
            load_fast(arg, next_pc);
            break;
        case STORE_FAST:
            store_fast(arg, next_pc);
            break;
        case JUMP_FORWARD:
            next_pc = jump_forward(arg, next_pc);
            break;
        case POP_JUMP_IF_FALSE:
            next_pc = pop_jump_if_false(arg, next_pc);
            break;
        case LOAD_GLOBAL:
            load_global(arg, next_pc);
            break;
        case CALL_FUNCTION:
            call_function(arg, next_pc);
            break;
        case MAKE_FUNCTION:
            make_function(arg, next_pc);
            break;
        case MAKE_CLOSURE:
            make_closure(arg, next_pc);
            break;
        case DUP_TOP:
            dup_top(arg, next_pc);
            break;
        case ROT_THREE:
            rot_three(arg, next_pc);
            break;
        case ROT_TWO:
            rot_two(arg, next_pc);
            break;
        case COMPARE_OP:
            compare_op(arg, next_pc);
            break;
        case JUMP_IF_FALSE_OR_POP:
            next_pc = jump_if_false_or_pop(arg, next_pc);
            break;
        case JUMP_IF_TRUE_OR_POP:
            next_pc = jump_if_true_or_pop(arg, next_pc);
            break;
        case BUILD_TUPLE:
            build_tuple(arg, next_pc);
            break;
        case SETUP_LOOP:
            setup_loop(arg, next_pc);
            break;
        case GET_ITER:
            get_iter(arg, next_pc);
            break;
        case FOR_ITER:
            next_pc = for_iter(arg, next_pc);
            break;
        case POP_BLOCK:
            _pop_block(arg, next_pc);
            break;
        case BREAK_LOOP:
            next_pc = break_loop(arg, next_pc);
            break;
        case UNARY_POSITIVE:
            unary_positive(arg, next_pc);
            break;
        case UNARY_NEGATIVE:
            unary_negative(arg, next_pc);
            break;
        case UNARY_NOT:
            unary_not(arg, next_pc);
            break;
        case UNARY_INVERT:
            unary_invert(arg, next_pc);
            break;
        case SETUP_FINALLY:
            setup_finally(arg, next_pc);
            break;
        case SETUP_EXCEPT:
            setup_except(arg, next_pc);
            break;
        case POP_EXCEPT:
            pop_except(arg, next_pc);
            break;
        case DELETE_FAST:
            delete_fast(arg, next_pc);
            break;
        case LOAD_ATTR:
            load_attr(arg, next_pc);
            break;
        case STORE_ATTR:
            store_attr(arg, next_pc);
            break;
        case DELETE_ATTR:
            delete_attr(arg, next_pc);
            break;
        case IMPORT_NAME:
            import_name(arg, next_pc);
            break;
        case STORE_GLOBAL:
            store_global(arg, next_pc);
            break;
        case LOAD_NAME:
            load_name(arg, next_pc);
            break;
        case STORE_NAME:
            store_name(arg, next_pc);
            break;
        case BINARY_SUBSCR:
            binary_subscr(arg, next_pc);
            break;
        case BUILD_LIST:
            build_list(arg, next_pc);
            break;
        case IMPORT_FROM:
            import_from(arg, next_pc);
            break;
        case IMPORT_STAR:
            import_star(arg, next_pc);
            break;
        case BUILD_SET:
            build_set(arg, next_pc);
            break;
        case LOAD_BUILD_CLASS:
            load_build_class(arg, next_pc);
            break;
        case BUILD_MAP:
            build_map(arg, next_pc);
            break;
        case LOAD_CLOSURE:
            load_closure(arg, next_pc);
            break;
        case LOAD_DEREF:
            load_deref(arg, next_pc);
            break;
        case STORE_DEREF:
            store_deref(arg, next_pc);
            break;
        case SETUP_WITH:
            setup_with(arg, next_pc);
            break;
        case WITH_CLEANUP:
            with_cleanup(arg, next_pc);
            break;
        case YIELD_VALUE:
            yield_value(arg, next_pc);
            break;
        case UNPACK_SEQUENCE:
            unpack_sequence(arg, next_pc);
            break;
        case STORE_SUBSCR:
            store_subscr(arg, next_pc);
            break;
        }
    }
}

M_BaseObject* PyFrame::get_locals() { return locals; }

void PyFrame::set_locals(M_BaseObject* locals) { this->locals = locals; }

FrameBlock* PyFrame::unwind_stack(int why)
{
    while (block_stack.size() > 0) {
        FrameBlock* block = pop_block();
        if (block->handling_mask() & why) {
            return block;
        }

        block->cleanup(this);
    }

    _finished_execution = true;
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

void PyFrame::fill_cellvars_from_args()
{
    const std::vector<int>& args_as_cells = pycode->args_as_cellvars();
    for (size_t i = 0; i < args_as_cells.size(); i++) {
        int idx = args_as_cells[i];
        if (idx != -1) {
            cells[i]->set(local_vars[idx]);
        }
    }
}

#define DEF_BINARY_OPER(name) DEF_BINARY_OPER_ALIAS(name, name)

#define DEF_BINARY_OPER_ALIAS(name, opname)               \
    void PyFrame::binary_##name(int arg, int next_pc)     \
    {                                                     \
        M_BaseObject* obj2 = pop_value();                 \
        M_BaseObject* obj1 = pop_value();                 \
        M_BaseObject* result = space->opname(obj1, obj2); \
        push_value(result);                               \
    }

DEF_BINARY_OPER(add)
DEF_BINARY_OPER(sub)
DEF_BINARY_OPER(mul)
DEF_BINARY_OPER(truediv)
DEF_BINARY_OPER(floordiv)
DEF_BINARY_OPER(mod)
DEF_BINARY_OPER(lshift)
DEF_BINARY_OPER(rshift)
DEF_BINARY_OPER(and_)
DEF_BINARY_OPER(or_)
DEF_BINARY_OPER(xor_)
DEF_BINARY_OPER(pow)

DEF_BINARY_OPER(inplace_add)
DEF_BINARY_OPER(inplace_sub)
DEF_BINARY_OPER(inplace_mul)
DEF_BINARY_OPER(inplace_truediv)
DEF_BINARY_OPER(inplace_floordiv)
DEF_BINARY_OPER(inplace_mod)
DEF_BINARY_OPER(inplace_lshift)
DEF_BINARY_OPER(inplace_rshift)
DEF_BINARY_OPER(inplace_and)
DEF_BINARY_OPER(inplace_or)
DEF_BINARY_OPER(inplace_xor)
DEF_BINARY_OPER(inplace_pow)

DEF_BINARY_OPER_ALIAS(subscr, getitem)

void PyFrame::pop_top(int arg, int next_pc) { pop_value(); }

void PyFrame::load_const(int arg, int next_pc) { push_value(get_const(arg)); }

void PyFrame::load_fast(int arg, int next_pc)
{
    M_BaseObject* value = local_vars[arg];
    if (!value) {
        throw InterpError::format(
            space, space->UnboundLocalError_type(),
            "local variable '%s' referenced before assignment",
            get_localname(arg).c_str());
    }
    push_value(value);
}

void PyFrame::store_fast(int arg, int next_pc)
{
    M_BaseObject* value = pop_value();
    local_vars[arg] = value;
}

void PyFrame::load_global(int arg, int next_pc)
{
    M_BaseObject* name = get_name(arg);
    std::string unwrapped_name = space->unwrap_str(name);

    M_BaseObject* value = space->finditem_str(globals, unwrapped_name);
    if (!value) {
        value = space->get_builtin()->get_dict_value(space, unwrapped_name);
        if (!value)
            throw InterpError::format(space, space->NameError_type(),
                                      "global name '%s' not found",
                                      unwrapped_name.c_str());
    }

    push_value(value);
}

void PyFrame::call_function_common(int arg, M_BaseObject* star,
                                   M_BaseObject* starstar)
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

            M_BaseObject* value = pop_value();
            M_BaseObject* key = pop_value();

            keywords[nkwargs] = space->unwrap_str(key);
            keyword_values[nkwargs] = value;
        }
    }

    pop_values(nargs, args);

    Arguments arguments(space, args, keywords, keyword_values);
    M_BaseObject* func = pop_value();
    M_BaseObject* result = space->call_args(context, func, arguments);

    push_value(result);
}

void PyFrame::call_function(int arg, int next_pc) { call_function_common(arg); }

void PyFrame::make_function(int arg, int next_pc)
{
    M_BaseObject* qualname = pop_value();
    M_BaseObject* code_obj = pop_value();

    int nr_defaults = arg & 0xff;
    std::vector<M_BaseObject*> defaults;

    if (nr_defaults > 0) pop_values(nr_defaults, defaults);

    PyCode* code = dynamic_cast<PyCode*>(code_obj);
    if (!code)
        throw InterpError(space->TypeError_type(),
                          space->wrap_str(context, "expected code object"));

    M_BaseObject* func = new (context) Function(space, code, defaults, globals);
    push_value(space->wrap(context, func));
}

void PyFrame::make_closure(int arg, int next_pc)
{
    M_BaseObject* qualname = pop_value();
    M_BaseObject* code_obj = pop_value();
    M_BaseObject* freevars_obj = pop_value();

    std::vector<M_BaseObject*> freevars;
    space->unwrap_tuple(freevars_obj, freevars);

    int nr_defaults = arg & 0xff;
    std::vector<M_BaseObject*> defaults;

    if (nr_defaults > 0) pop_values(nr_defaults, defaults);

    PyCode* code = dynamic_cast<PyCode*>(code_obj);
    if (!code)
        throw InterpError(space->TypeError_type(),
                          space->wrap_str(context, "expected code object"));

    Function* func = new (context) Function(space, code, defaults, globals);
    func->set_closure(freevars);

    push_value(space->wrap(context, func));
}

int PyFrame::jump_absolute(int arg) { return arg; }

int PyFrame::jump_forward(int arg, int next_pc)
{
    next_pc += arg;
    return next_pc;
}

int PyFrame::pop_jump_if_false(int arg, int next_pc)
{
    M_BaseObject* value = pop_value();
    if (!space->is_true(value)) {
        next_pc = arg;
    }

    return next_pc;
}

void PyFrame::dup_top(int arg, int next_pc) { push_value(peek_value()); }

void PyFrame::rot_three(int arg, int next_pc)
{
    M_BaseObject* v1 = pop_value();
    M_BaseObject* v2 = pop_value();
    M_BaseObject* v3 = pop_value();
    push_value(v1);
    push_value(v3);
    push_value(v2);
}

void PyFrame::compare_op(int arg, int next_pc)
{
    M_BaseObject* v2 = pop_value();
    M_BaseObject* v1 = pop_value();

    M_BaseObject* result;
    switch (arg) {
    case 0: /* < */
        result = space->lt(v1, v2);
        break;
    case 1: /* <= */
        result = space->le(v1, v2);
        break;
    case 2: /* == */
        result = space->eq(v1, v2);
        break;
    case 3: /* != */
        result = space->ne(v1, v2);
        break;
    case 4: /* > */
        result = space->gt(v1, v2);
        break;
    case 5: /* >= */
        result = space->ge(v1, v2);
        break;
    case 6: /* in */
        result = space->contains(v2, v1);
        break;
    case 7: /* not in */
        result = space->not_(space->contains(v2, v1));
        break;
        break;
    case 8: /* is */
        result = space->new_bool(space->i_is(v1, v2));
        break;
    case 9: /* is not */
        result = space->new_bool(!space->i_is(v1, v2));
        break;
    case 10:
        result = space->new_bool(space->match_exception(v1, v2));
        break;
    }
    push_value(result);
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

int PyFrame::jump_if_true_or_pop(int arg, int next_pc)
{
    M_BaseObject* value = peek_value();
    int result;
    if (space->is_true(value))
        result = arg;
    else {
        pop_value();
        result = next_pc;
    }

    return result;
}

void PyFrame::rot_two(int arg, int next_pc)
{
    M_BaseObject* v1 = pop_value();
    M_BaseObject* v2 = pop_value();
    push_value(v1);
    push_value(v2);
}

void PyFrame::build_tuple(int arg, int next_pc)
{
    std::vector<M_BaseObject*> args;

    pop_values(arg, args);
    M_BaseObject* tup = space->new_tuple(context, args);
    push_value(tup);
}

void PyFrame::setup_loop(int arg, int next_pc)
{
    FrameBlock* loop_block = new LoopBlock(next_pc + arg, value_stack.size());
    push_block(loop_block);
}

void PyFrame::get_iter(int arg, int next_pc)
{
    M_BaseObject* iterable = pop_value();
    M_BaseObject* iterator = space->iter(iterable);
    push_value(iterator);
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

#define DEF_UNARY_OPER(opname, name)                   \
    void PyFrame::unary_##opname(int arg, int next_pc) \
    {                                                  \
        M_BaseObject* obj = pop_value();               \
        M_BaseObject* result = space->name(obj);       \
        push_value(result);                            \
    }

DEF_UNARY_OPER(positive, pos)
DEF_UNARY_OPER(negative, neg)
DEF_UNARY_OPER(invert, invert)
DEF_UNARY_OPER(not, not_)

void PyFrame::setup_finally(int arg, int next_pc)
{
    FrameBlock* finally_block =
        new FinallyBlock(next_pc + arg, value_stack.size());
    push_block(finally_block);
}

void PyFrame::setup_except(int arg, int next_pc)
{
    FrameBlock* except_block =
        new ExceptBlock(next_pc + arg, value_stack.size());
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
        throw InterpError::format(
            space, space->UnboundLocalError_type(),
            "local variable '%s' referenced before assignment",
            get_localname(arg).c_str());
    }
    local_vars[arg] = nullptr;
}

M_BaseObject* PyFrame::end_finally()
{
    M_BaseObject* top = pop_value();
    if (space->i_is(top, space->wrap_None())) return nullptr;

    /* stack : [unwinder]           case of a finally */
    StackUnwinder* as_unwinder = dynamic_cast<StackUnwinder*>(top);
    if (as_unwinder) return top;

    /* stack : [ExceptionUnwinder, value, type]		case of an except */
    pop_value(); /* pop value */
    return pop_value();
}

void PyFrame::load_attr(int arg, int next_pc)
{
    M_BaseObject* obj = pop_value();
    M_BaseObject* attr = get_name(arg);
    M_BaseObject* value = space->getattr(obj, attr);
    push_value(value);
}
void PyFrame::store_attr(int arg, int next_pc)
{
    M_BaseObject* obj = pop_value();
    M_BaseObject* attr = get_name(arg);
    M_BaseObject* value = pop_value();
    space->setattr(obj, attr, value);
}

void PyFrame::delete_attr(int arg, int next_pc)
{
    M_BaseObject* obj = pop_value();
    M_BaseObject* attr = get_name(arg);
    space->delattr(obj, attr);
}

void PyFrame::import_name(int arg, int next_pc)
{
    M_BaseObject* mod_name = get_name(arg);
    M_BaseObject* from_list = pop_value();
    M_BaseObject* level = pop_value();

    M_BaseObject* import_func =
        space->get_builtin()->get_dict_value(space, "__import__");
    if (!import_func) {
        throw InterpError(space->ImportError_type(),
                          space->wrap_str(context, "__import__ not found"));
    }

    M_BaseObject* wrapped_locals = locals;
    if (!wrapped_locals) wrapped_locals = space->wrap_None();

    M_BaseObject* wrapped_globals = globals;
    M_BaseObject* obj = space->call_function(
        context, import_func,
        {mod_name, wrapped_globals, wrapped_locals, from_list, level});

    push_value(obj);
}

void PyFrame::store_global(int arg, int next_pc)
{
    M_BaseObject* w_name = get_name(arg);
    std::string name = space->unwrap_str(w_name);
    M_BaseObject* value = pop_value();
    space->setitem_str(globals, name, value);
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
        if (!value)
            throw InterpError::format(space, space->NameError_type(),
                                      "name '%s' not found", name.c_str());
    }

    push_value(value);
}

void PyFrame::store_name(int arg, int next_pc)
{
    M_BaseObject* w_name = get_name(arg);
    std::string name = space->unwrap_str(w_name);
    M_BaseObject* value = pop_value();
    space->setitem_str(locals, name, value);
}

void PyFrame::build_list(int arg, int next_pc)
{
    std::vector<M_BaseObject*> args;
    pop_values(arg, args);
    M_BaseObject* list = space->new_list(context, args);
    push_value(list);
}

void PyFrame::import_from(int arg, int next_pc)
{
    M_BaseObject* w_name = get_name(arg);
    M_BaseObject* module = peek_value();
    M_BaseObject* value;

    try {
        value = space->getattr(module, w_name);
    } catch (InterpError& exc) {
        if (!exc.match(space, space->AttributeError_type())) throw exc;
        throw InterpError::format(space, space->ImportError_type(),
                                  "cannot import name '%s'",
                                  space->unwrap_str(w_name).c_str());
    }
    push_value(value);
}

void PyFrame::import_star(int arg, int next_pc)
{
    M_BaseObject* module = pop_value();
    M_BaseObject* locals = get_locals();
    M_BaseObject *dict, *all;
    bool skip_leading_underscores = false;

    try {
        all = space->getattr_str(module, "__all__");
    } catch (InterpError& exc) {
        if (!exc.match(space, space->AttributeError_type())) throw exc;
        try {
            dict = space->getattr_str(module, "__dict__");
        } catch (InterpError& exc) {
            if (!exc.match(space, space->AttributeError_type())) throw exc;

            throw InterpError(
                space->ImportError_type(),
                space->wrap_str(
                    context,
                    "from-import-* object has no __dict__ and no __all__"));
        }

        M_BaseObject* keys_impl = space->getattr_str(dict, "keys");
        all = space->call_function(context, keys_impl, {});

        skip_leading_underscores = true;
    }

    M_BaseObject* iter = space->iter(all);
    while (true) {
        try {
            M_BaseObject* name = space->next(iter);
            std::string unwrapped = space->unwrap_str(name);
            if (skip_leading_underscores && unwrapped[0] == '_') {
                continue;
            }
            M_BaseObject* value = space->getattr(module, name);
            space->setitem(locals, name, value);
        } catch (InterpError& e) {
            if (!e.match(space, space->StopIteration_type())) throw e;
            break;
        }
    }

    set_locals(locals);
}

void PyFrame::build_set(int arg, int next_pc)
{
    std::vector<M_BaseObject*> args;
    pop_values(arg, args);
    M_BaseObject* set = space->new_set(context);
    M_BaseObject* add_impl = space->lookup(set, "add");
    for (auto& item : args) {
        space->call_function(context, add_impl, {set, item});
    }
    push_value(set);
}

void PyFrame::load_build_class(int arg, int next_pc)
{
    M_BaseObject* value =
        space->get_builtin()->get_dict_value(space, "__build_class__");
    if (!value) {
        throw InterpError(
            space->ImportError_type(),
            space->wrap_str(context, "__build_class__ not found"));
    }

    push_value(value);
}

void PyFrame::build_map(int arg, int next_pc)
{
    M_BaseObject* dict = space->new_dict(context);
    push_value(dict);
}

void PyFrame::load_closure(int arg, int next_pc)
{
    M_BaseObject* cell = cells[arg];
    push_value(space->wrap(context, cell));
}

void PyFrame::throw_unbound_error(int index)
{
    std::string varname;
    std::string error_msg;
    M_BaseObject* error_type = nullptr;
    int ncells = pycode->get_ncellvars();
    if (index < ncells) {
        varname = pycode->get_cellvars()[index];
        error_msg = "local variable '";
        error_msg = error_msg + varname;
        error_msg = error_msg + "' referenced before assignment";
        error_type = space->UnboundLocalError_type();
    } else {
        varname = pycode->get_freevars()[index - ncells];
        error_msg = "free variable '";
        error_msg = error_msg + varname;
        error_msg =
            error_msg + "' referenced before assignment in enclosing scope";
        error_type = space->NameError_type();
    }

    throw InterpError(error_type, space->wrap_str(context, error_msg));
}

void PyFrame::load_deref(int arg, int next_pc)
{
    Cell* cell = cells[arg];
    M_BaseObject* value = cell->get();
    if (!value) {
        throw_unbound_error(arg);
    }
    push_value(value);
}

void PyFrame::store_deref(int arg, int next_pc)
{
    M_BaseObject* value = pop_value();
    Cell* cell = cells[arg];
    cell->set(value);
}

void PyFrame::setup_with(int arg, int next_pc)
{
    M_BaseObject* manager = peek_value();
    M_BaseObject* enter = space->lookup(manager, "__enter__");
    M_BaseObject* descr = space->lookup(manager, "__exit__");

    if (!enter || !descr) {
        throw InterpError::format(
            space, space->AttributeError_type(),
            "'%s' object is not a context manager (no __enter__/"
            "__exit__ method)",
            space->get_type_name(manager).c_str());
    }

    M_BaseObject* exit_func = space->get(descr, manager);

    manager = pop_value();
    push_value(exit_func);
    M_BaseObject* result =
        space->get_and_call_function(context, enter, {manager});

    FrameBlock* frame = new WithBlock(next_pc + arg, value_stack.size());
    push_block(frame);

    push_value(result);
}

void PyFrame::with_cleanup(int arg, int next_pc)
{
    StackUnwinder* unwinder = static_cast<StackUnwinder*>(pop_value());
    M_BaseObject* exit_func = pop_value();
    push_value(unwinder);

    if (unwinder->why() == WhyCode::WHY_EXCEPTION) {
        /* TODO: handle exception */

    } else {
        space->call_function(
            context, exit_func,
            {space->wrap_None(), space->wrap_None(), space->wrap_None()});
    }
}

void PyFrame::yield_value(int arg, int next_pc) { throw YieldException(); }

void PyFrame::unpack_sequence(int arg, int next_pc)
{
    M_BaseObject* iterable = pop_value();

    std::vector<M_BaseObject*> v;
    space->unwrap_tuple(iterable, v);
    for (auto it = v.rbegin(); it != v.rend(); it++) {
        push_value(*it);
    }
}

void PyFrame::store_subscr(int arg, int next_pc)
{
    ObjSpace* space = context->get_space();

    M_BaseObject* subscr = pop_value();
    M_BaseObject* obj = pop_value();
    M_BaseObject* value = pop_value();

    space->setitem(obj, subscr, value);
}
