#ifndef _INTERPRETER_ARGUMENTS_H_
#define _INTERPRETER_ARGUMENTS_H_

#include "objects/base_object.h"
#include <deque>
#include <initializer_list>

namespace mtpython {
namespace interpreter {

/* List of arguments to a function call */
class Arguments {
protected:
	std::deque<objects::M_BaseObject*> args;
public:
	Arguments(std::initializer_list<objects::M_BaseObject*>& args) : args(args) { }

	void prepend(objects::M_BaseObject* arg) { args.push_front(arg); }
	std::size_t size() { return args.size(); }
	objects::M_BaseObject* operator[](std::size_t idx) { return args[idx]; }
};

}
}

#endif /* _INTERPRETER_ARGUMENTS_H_ */
