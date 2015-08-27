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

	std::vector<std::string> keywords;
	std::vector<objects::M_BaseObject*> keyword_values;
public:
	Arguments(objects::ObjSpace* space, std::initializer_list<objects::M_BaseObject*>& args) : space(space), args(args) { }
	Arguments(objects::ObjSpace* space, std::vector<objects::M_BaseObject*>& args) : space(space), args(args) { }
	Arguments(objects::ObjSpace* space, std::vector<objects::M_BaseObject*>& args, 
		std::vector<std::string>& keywords, std::vector<objects::M_BaseObject*>& keyword_values) : 
		space(space), args(args), keywords(keywords), keyword_values(keyword_values) { }

	void parse(const std::string& fname, objects::M_BaseObject* first, Signature& sig, std::vector<objects::M_BaseObject*>& scope) {
		std::vector<objects::M_BaseObject*> defaults;
		parse(fname, first, sig, scope, defaults);
	}

	void parse(const std::string& fname, objects::M_BaseObject* first, Signature& sig, std::vector<objects::M_BaseObject*>& scope, std::vector<objects::M_BaseObject*>& defaults);
};

}
}

#endif /* _INTERPRETER_ARGUMENTS_H_ */
