#include "interpreter/arguments.h"
#include "objects/obj_space.h"
#include "interpreter/error.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;

void Arguments::parse(M_BaseObject* first, Signature& sig, std::vector<M_BaseObject*>& scope, std::vector<M_BaseObject*>& defaults)
{
	int argcount = sig.get_nargs();

	scope.resize(sig.scope_len(), space->wrap_None());

	/* place the first object */
	int front = 0;
	if (first) {
		front = 1;
		if (argcount > 0) scope[0] = first;
	}

	int nargs = args.size();
	int args_avail = nargs + front;

	/* place positional arguments */
	int input_argcount = front;
	if (input_argcount < argcount) {
		int take = (argcount - front) < nargs ? (argcount - front) : nargs;
		for (int i = 0; i < take; i++) {
			scope[input_argcount + i] = args[i];
		}

		input_argcount += take;
	}

	/* extra positional arguments into vararg */
	if (sig.has_vararg()) {
		std::vector<M_BaseObject*> starargs;
		if (front > argcount) {	/* collect all arguments into vararg */
			starargs.push_back(first);
			if (nargs > 0) {
				starargs.insert(starargs.end(), args.begin(), args.end());
			}
		} else {
			int left = argcount - front;
			if (nargs > left) {
				if (sig.get_varargname() == "*") {	
					/* if the next unfilled slot is a vararg slot, and it does
           				not have a name, then it is an error. */
					throw InterpError::format(space, space->TypeError_type(), "function takes %d arguments but %d arguments were given", argcount, args_avail);
				}
				starargs.insert(starargs.end(), args.begin() + left, args.end());
			}
			else 
				starargs.clear();
		}
		M_BaseObject* wrapped_starargs = space->new_tuple(starargs);
		scope[argcount] = wrapped_starargs;
	} else if (args_avail > argcount) {	/* error */
		throw InterpError::format(space, space->TypeError_type(), "function takes %d arguments but %d arguments were given", argcount, args_avail);
	}
}
