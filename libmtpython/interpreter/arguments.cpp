#include "interpreter/arguments.h"
#include "objects/obj_space.h"
#include "interpreter/error.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;

/* TODO: support keyword only argument(PEP 3102) */
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
	int nkwargs = keywords.size();
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

	/* create kwarg dict */
	M_BaseObject* wrapped_kwargs = nullptr;
	if (sig.has_kwarg()) {
		wrapped_kwargs = space->new_dict();
		scope[argcount + sig.has_vararg() ? 1 : 0] = wrapped_kwargs;
	}

	/* match keywords */
	std::vector<int> kwarg_index;	/* scope index -> keyword_values index */
	if (nkwargs > 0) {
		kwarg_index.resize(argcount - input_argcount, -1);

		/* calculate keyword arg index */
		int remaining_kwargs = nkwargs;
		for (int i = 0; i < nkwargs; i++) {
			std::string& keyword_name = keywords[i];

			int index = sig.find_argname(keyword_name);

			/* multiple values */
			if (index < input_argcount) {
				if (index != -1) throw InterpError::format(space, space->TypeError_type(), "got multiple values for argument '%s'", keyword_name.c_str());
			} else {
				kwarg_index[index - input_argcount] = i;
				remaining_kwargs--;
			}
		}

		/* collect extra keyword arguments into kwarg */
		if (remaining_kwargs > 0) {
			if (wrapped_kwargs) {
				for (std::size_t i = 0; i < keywords.size(); i++) {
					bool matched = false;
					for (std::size_t j : kwarg_index) if (matched = (i == j)) break;
					if (!matched) {	/* extra keyword */
						space->setitem_str(wrapped_kwargs, keywords[i], keyword_values[i]);
					}
				}
			} else {
				throw InterpError(space->TypeError_type(), space->wrap_str("got unexpected keyword arguments"));
			}
		}
	}

	if (input_argcount < argcount) {
		for (std::size_t i = input_argcount; i < (std::size_t)argcount; i++) {
			if (kwarg_index.size() > 0) {
				int index = kwarg_index[i - input_argcount];
				if (index >= 0) scope[i] = keyword_values[index];
			}
		}
	}
}
