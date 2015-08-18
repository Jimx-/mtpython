#ifndef _INTERPRETER_PYFRAME_H_
#define _INTERPRETER_PYFRAME_H_

#include "objects/base_object.h"
#include "interpreter/code.h"
#include "interpreter/pycode.h"
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
	
	int pc;

	void push_value(mtpython::objects::M_BaseObject* value) 
	{ 
		value_stack.push(value); 
		context->gc_track_object(value);
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

	mtpython::objects::M_BaseObject* get_const(int index);

	virtual void load_const(int arg, int next_pc);
	virtual void binary_add(int arg, int next_pc);
public:
	PyFrame(vm::ThreadContext* context, Code* code, mtpython::objects::M_BaseObject* globals);

	objects::M_BaseObject* exec();
	objects::M_BaseObject* execute_frame();

	objects::M_BaseObject* dispatch(vm::ThreadContext* context, Code* code, int next_pc);
	int execute_bytecode(vm::ThreadContext* context, std::vector<char>& bytecode, int next_pc);
	int dispatch_bytecode(vm::ThreadContext* context, std::vector<char>& bytecode, int next_pc);
};

}
}

#endif /* _INTERPRETER_PYFRAME_H_ */
