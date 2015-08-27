#ifndef _INTERPRETER_PYFRAME_H_
#define _INTERPRETER_PYFRAME_H_

#include "objects/base_object.h"
#include "interpreter/code.h"
#include "interpreter/pycode.h"
#include "interpreter/error.h"
#include "vm/vm.h"
#include <string>
#include <stack>

namespace mtpython {
namespace interpreter {

class PyFrame : public mtpython::objects::M_BaseObject {
protected:
	vm::ThreadContext* context;
	PyCode* pycode;
	mtpython::objects::M_BaseObject* globals;

	std::stack<mtpython::objects::M_BaseObject*> value_stack;
	std::vector<mtpython::objects::M_BaseObject*> local_vars;
	
	int pc;

	void push_value(mtpython::objects::M_BaseObject* value) 
	{ 
		value_stack.push(value); 
		if (value) context->gc_track_object(value);
	}

	mtpython::objects::M_BaseObject* pop_value() 
	{ 
		mtpython::objects::M_BaseObject* tmp = value_stack.top(); 
		value_stack.pop(); 
		return tmp; 
	}

	mtpython::objects::M_BaseObject* pop_value_untrack() 
	{
		mtpython::objects::M_BaseObject* tmp = value_stack.top(); 
		context->gc_untrack_object(tmp);
		value_stack.pop();
		return tmp; 
	}

	void pop_values_untrack(int n, std::vector<mtpython::objects::M_BaseObject*>& v) 
	{
		v.resize(n);
		n--;
		while (n >= 0) {
			mtpython::objects::M_BaseObject* tmp = value_stack.top(); 
			context->gc_untrack_object(tmp);
			value_stack.pop();
			v[n--] = tmp;
		}
	}

	mtpython::objects::M_BaseObject* peek_value()
	{
		return value_stack.top();
	}

	mtpython::objects::M_BaseObject* get_const(int index);
	mtpython::objects::M_BaseObject* get_name(int index);

	int handle_interp_error(InterpError& exc);

	virtual void pop_top(int arg, int next_pc);
	virtual void load_const(int arg, int next_pc);
	virtual void binary_add(int arg, int next_pc);
	virtual void load_fast(int arg, int next_pc);
	virtual void store_fast(int arg, int next_pc);
	virtual void load_global(int arg, int next_pc);
	virtual void call_function(int arg, int next_pc);
	virtual void make_function(int arg, int next_pc);
	virtual int jump_absolute(int arg);
	virtual int jump_forward(int arg, int next_pc);
	virtual int pop_jump_if_false(int arg, int next_pc);
	virtual void dup_top(int arg, int next_pc);
	virtual void rot_two(int arg, int next_pc);
	virtual void rot_three(int arg, int next_pc);
	virtual void compare_op(int arg, int next_pc);
	virtual int jump_if_false_or_pop(int arg, int next_pc);

	virtual void call_function_common(int arg, mtpython::objects::M_BaseObject* star=nullptr, mtpython::objects::M_BaseObject* starstar=nullptr);
public:
	PyFrame(vm::ThreadContext* context, Code* code, mtpython::objects::M_BaseObject* globals);

	objects::M_BaseObject* exec();
	objects::M_BaseObject* execute_frame();

	std::vector<mtpython::objects::M_BaseObject*>& get_local_vars() { return local_vars; }

	objects::M_BaseObject* dispatch(vm::ThreadContext* context, Code* code, int next_pc);
	int execute_bytecode(vm::ThreadContext* context, std::vector<unsigned char>& bytecode, int next_pc);
	int dispatch_bytecode(vm::ThreadContext* context, std::vector<unsigned char>& bytecode, int next_pc);
};

}
}

#endif /* _INTERPRETER_PYFRAME_H_ */
