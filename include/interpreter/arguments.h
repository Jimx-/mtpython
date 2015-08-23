#ifndef _INTERPRETER_ARGUMENTS_H_
#define _INTERPRETER_ARGUMENTS_H_

#include "objects/base_object.h"
#include "interpreter/signature.h"
#include <vector>
#include <initializer_list>

namespace mtpython {

namespace objects {
class ObjSpace;
}

namespace interpreter {

/* List of arguments to a function call */
class Arguments {
protected:
	objects::ObjSpace* space;
	std::vector<objects::M_BaseObject*> args;
public:
	Arguments(objects::ObjSpace* space, std::initializer_list<objects::M_BaseObject*>& args) : space(space), args(args) { }
	Arguments(objects::ObjSpace* space, std::vector<objects::M_BaseObject*>& args) : space(space), args(args) { }

	void parse(objects::M_BaseObject* first, Signature& sig, std::vector<objects::M_BaseObject*>& scope) {
		std::vector<objects::M_BaseObject*> defaults;
		parse(first, sig, scope, defaults);
	}

	void parse(objects::M_BaseObject* first, Signature& sig, std::vector<objects::M_BaseObject*>& scope, std::vector<objects::M_BaseObject*>& defaults);
};

}
}

#endif /* _INTERPRETER_ARGUMENTS_H_ */
