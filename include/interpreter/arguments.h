#ifndef _INTERPRETER_ARGUMENTS_H_
#define _INTERPRETER_ARGUMENTS_H_

#include "objects/base_object.h"
#include "interpreter/signature.h"
#include <vector>
#include <deque>
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
	std::deque<objects::M_BaseObject*> args;

	std::vector<std::string> keywords;
	std::vector<objects::M_BaseObject*> keyword_values;
public:
	Arguments(objects::ObjSpace* space, const std::initializer_list<objects::M_BaseObject*>& args) : space(space), args(args) { }
	Arguments(objects::ObjSpace* space, const std::vector<objects::M_BaseObject*>& args) : space(space), args(args.begin(), args.end()) { }
	Arguments(objects::ObjSpace* space, const std::vector<objects::M_BaseObject*>& args,
			  const std::vector<std::string>& keywords, const std::vector<objects::M_BaseObject*>& keyword_values) :
		space(space), args(args.begin(), args.end()), keywords(keywords), keyword_values(keyword_values) { }

	objects::M_BaseObject* front() const { return args.front(); }
	void pop_front() { args.pop_front(); }
	void prepend(objects::M_BaseObject* obj) { args.push_front(obj); }
	
	void parse(const std::string& fname, objects::M_BaseObject* first, Signature& sig, std::vector<objects::M_BaseObject*>& scope) const {
		std::vector<objects::M_BaseObject*> defaults;
		parse(fname, first, sig, scope, defaults);
	}

	void parse(const std::string& fname, objects::M_BaseObject* first, Signature& sig, std::vector<objects::M_BaseObject*>& scope, const std::vector<objects::M_BaseObject*>& defaults) const;
};

}
}

#endif /* _INTERPRETER_ARGUMENTS_H_ */
