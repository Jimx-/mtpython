#ifndef _INTERPRETER_PYFRAME_H_
#define _INTERPRETER_PYFRAME_H_

#include <string>
#include <stack>

#include "objects/base_object.h"
#include "objects/obj_space.h"
#include "interpreter/code.h"
#include "interpreter/pycode.h"
#include "interpreter/error.h"
#include "interpreter/cell.h"
#include "vm/vm.h"
#include "exceptions.h"

namespace mtpython {
namespace interpreter {

typedef enum {
    WHY_NOT =       0x0001, /* No error */
        WHY_EXCEPTION = 0x0002, /* Exception occurred */
        WHY_RETURN =    0x0008, /* 'return' statement */
        WHY_BREAK =     0x0010, /* 'break' statement */
        WHY_CONTINUE =  0x0020, /* 'continue' statement */
        WHY_YIELD =     0x0040, /* 'yield' operator */
        WHY_SILENCED =  0x0080  /* Exception silenced by 'with' */
} WhyCode;

class StackUnwinder : public mtpython::objects::M_BaseObject {
protected:
    WhyCode why_code;
public:
    WhyCode why() { return why_code; }
	objects::M_BaseObject* unhandle() { throw NotImplementedException("Abstract"); }
};

class ExceptionUnwinder : public StackUnwinder {
private:
    InterpError error;
public:
    ExceptionUnwinder(const InterpError& e) : error(e) { why_code = WHY_EXCEPTION; }
    const InterpError& get_error() { return error; }
};

class ReturnUnwinder : public StackUnwinder {
private:
    objects::M_BaseObject* retval;
public:
    ReturnUnwinder(objects::M_BaseObject* ret) : retval(ret) { why_code = WHY_RETURN; }
};

class BreakUnwinder : public StackUnwinder {
public:
    BreakUnwinder() { why_code = WHY_BREAK; }
};

class FrameBlock {
protected:
    int handler;
    int level;
    int mask;
public:
    FrameBlock(int handler, int level) : handler(handler), level(level) { mask = WHY_NOT; }

    int handling_mask() { return mask; }
    virtual int handle(PyFrame* frame, StackUnwinder* unwinder) { throw mtpython::NotImplementedException("Abstract"); }

    void cleanup(PyFrame* frame);
};

class LoopBlock : public FrameBlock {
public:
    LoopBlock(int handler, int level) : FrameBlock(handler, level) { mask = (WHY_BREAK | WHY_CONTINUE); };

    virtual int handle(PyFrame* frame, StackUnwinder* unwinder);
};

class ExceptBlock : public FrameBlock {
public:
    ExceptBlock(int handler, int level) : FrameBlock(handler, level) { mask = WHY_EXCEPTION; };

    virtual int handle(PyFrame* frame, StackUnwinder* unwinder);
};

class ExceptHandlerBlock : public FrameBlock {  /* Pseudo block for POP_EXCEPT */
public:
    ExceptHandlerBlock(int handler, int level) : FrameBlock(handler, level) { mask = 0; };
};

class FinallyBlock : public FrameBlock {
public:
    FinallyBlock(int handler, int level) : FrameBlock(handler, level) { mask = -1; };

    virtual int handle(PyFrame* frame, StackUnwinder* unwinder);
};

class WithBlock : public FinallyBlock {
public:
    WithBlock(int handler, int level) : FinallyBlock(handler, level) { }
};

class PyFrame : public mtpython::objects::M_BaseObject {
private:
    void init_cells(mtpython::objects::M_BaseObject* outer, PyCode* code);
    void throw_unbound_error(int index);

protected:
    vm::ThreadContext* context;
    objects::ObjSpace* space;
    PyCode* pycode;
    mtpython::objects::M_BaseObject* globals;
    mtpython::objects::M_BaseObject* locals;

    std::stack<mtpython::objects::M_BaseObject*> value_stack;
    std::vector<mtpython::objects::M_BaseObject*> local_vars;
    std::vector<mtpython::interpreter::Cell*> cells;
    std::stack<FrameBlock*> block_stack;

    int pc;
    bool _finished_execution;

    mtpython::objects::M_BaseObject* pop_value()
    {
        mtpython::objects::M_BaseObject* tmp = value_stack.top();
        value_stack.pop();
        return tmp;
    }

    mtpython::objects::M_BaseObject* pop_value_untrack()
    {
        mtpython::objects::M_BaseObject* tmp = value_stack.top();
        context->new_local_ref(tmp);
        value_stack.pop();
        return tmp;
    }

    void pop_values_untrack(int n, std::vector<mtpython::objects::M_BaseObject*>& v)
    {
        v.resize(n);
        n--;
        while (n >= 0) {
            mtpython::objects::M_BaseObject* tmp = value_stack.top();
            context->new_local_ref(tmp);
            value_stack.pop();
            v[n--] = tmp;
        }
    }

    mtpython::objects::M_BaseObject* peek_value()
    {
        return value_stack.top();
    }

    FrameBlock* pop_block()
    {
        FrameBlock* block = block_stack.top();
        block_stack.pop();

        return block;
    }

    mtpython::objects::M_BaseObject* get_const(int index);
    mtpython::objects::M_BaseObject* get_name(int index);
    const std::string& get_localname(int index);

    int handle_interp_error(InterpError& exc);

    FrameBlock* unwind_stack(int why);
    int unwind_stack_jump(StackUnwinder* unwinder);

    void pop_top(int arg, int next_pc);
    void load_const(int arg, int next_pc);
    void binary_add(int arg, int next_pc);
    void binary_sub(int arg, int next_pc);
    void binary_mul(int arg, int next_pc);
    void load_fast(int arg, int next_pc);
    void store_fast(int arg, int next_pc);
    void load_global(int arg, int next_pc);
    void call_function(int arg, int next_pc);
    void make_function(int arg, int next_pc);
    void make_closure(int arg, int next_pc);
    int jump_absolute(int arg);
    int jump_forward(int arg, int next_pc);
    int pop_jump_if_false(int arg, int next_pc);
    void dup_top(int arg, int next_pc);
    void rot_two(int arg, int next_pc);
    void rot_three(int arg, int next_pc);
    void compare_op(int arg, int next_pc);
    int jump_if_false_or_pop(int arg, int next_pc);
    void build_tuple(int arg, int next_pc);
    void setup_loop(int arg, int next_pc);
    void get_iter(int arg, int next_pc);
    int for_iter(int arg, int next_pc);
    void _pop_block(int arg, int next_pc);
    int break_loop(int arg, int next_pc);
    void unary_positive(int arg, int next_pc);
    void unary_negative(int arg, int next_pc);
    void unary_not(int arg, int next_pc);
    void unary_invert(int arg, int next_pc);
    void setup_except(int arg, int next_pc);
    void setup_finally(int arg, int next_pc);
    void pop_except(int arg, int next_pc);
    void delete_fast(int arg, int next_pc);
    void load_attr(int arg, int next_pc);
    void store_attr(int arg, int next_pc);
    void delete_attr(int arg, int next_pc);
    void import_name(int arg, int next_pc);
    void store_global(int arg, int next_pc);
    void load_name(int arg, int next_pc);
    void store_name(int arg, int next_pc);
    void binary_getitem(int arg, int next_pc);
    void binary_subscr(int arg, int next_pc);
    void build_list(int arg, int next_pc);
	void import_from(int arg, int next_pc);
	void import_star(int arg, int next_pc);
    void binary__and(int arg, int next_pc);
    void build_set(int arg, int next_pc);
    void load_build_class(int arg, int next_pc);
    void build_map(int arg, int next_pc);
    void load_closure(int arg, int next_pc);
    void load_deref(int arg, int next_pc);
    void store_deref(int arg, int next_pc);
    void setup_with(int arg, int next_pc);
    void with_cleanup(int arg, int next_pc);
    void yield_value(int arg, int next_pc);

    objects::M_BaseObject* end_finally();

    void call_function_common(int arg, mtpython::objects::M_BaseObject* star=nullptr, mtpython::objects::M_BaseObject* starstar=nullptr);
public:
    PyFrame(vm::ThreadContext* context, Code* code, mtpython::objects::M_BaseObject* globals, mtpython::objects::M_BaseObject* outer);

    void push_value(mtpython::objects::M_BaseObject* value)
    {
        value_stack.push(value);
        if (value) context->delete_local_ref(value);
    }

    int value_stack_level() { return value_stack.size(); }

    void push_block(FrameBlock* block)
    {
        block_stack.push(block);
    }

    vm::ThreadContext* get_context() { return context; }
    objects::ObjSpace* get_space() { return space; }
    objects::M_BaseObject* get_globals() { return globals; }
	objects::M_BaseObject* get_locals();
    int get_pc() { return pc; }
    bool finished_execution() { return _finished_execution; }
	void set_locals(objects::M_BaseObject* locals);
    PyCode* get_pycode() { return pycode; }

    objects::M_BaseObject* exec();
    objects::M_BaseObject* execute_frame(objects::M_BaseObject* arg = nullptr);

    std::vector<mtpython::objects::M_BaseObject*>& get_local_vars() { return local_vars; }
    const std::vector<mtpython::interpreter::Cell*>& get_cells() { return cells; }

    void fill_cellvars_from_args();

    objects::M_BaseObject* dispatch(vm::ThreadContext* context, Code* code, int next_pc);
    int execute_bytecode(vm::ThreadContext* context, std::vector<unsigned char>& bytecode, int next_pc);
    int dispatch_bytecode(vm::ThreadContext* context, std::vector<unsigned char>& bytecode, int next_pc);

    void drop_values_until(int level)
    {
        while (value_stack.size() > (std::size_t)level) pop_value();
    }
};

}
}

#endif /* _INTERPRETER_PYFRAME_H_ */
