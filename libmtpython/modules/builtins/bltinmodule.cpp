#include "modules/builtins/bltinmodule.h"
#include "objects/bltin_exceptions.h"
#include "interpreter/gateway.h"
#include "interpreter/signature.h"
#include "vm/vm.h"

#include <iostream>

using namespace mtpython::modules;
using namespace mtpython::objects;
using namespace mtpython::interpreter;

static M_BaseObject* builtin___import__(mtpython::vm::ThreadContext* context, std::vector<M_BaseObject*>& args)
{
	return nullptr;
}

/* The famous print() */
static M_BaseObject* builtin_print(mtpython::vm::ThreadContext* context, M_BaseObject* args, M_BaseObject* kwargs)
{
	std::vector<M_BaseObject*> values;
	ObjSpace* space = context->get_space();

	M_BaseObject* wrapped_seq = space->getitem_str(kwargs, std::string("seq"));
	std::string seq = wrapped_seq ? space->unwrap_str(wrapped_seq) : " ";

	M_BaseObject* wrapped_end = space->getitem_str(kwargs, std::string("end"));
	std::string end = wrapped_end ? space->unwrap_str(wrapped_end) : "\n";

	space->unwrap_tuple(args, values);

	for (std::size_t i = 0; i < values.size(); i++) {
		if (i > 0) std::cout << seq;
		std::cout << values[i]->to_string(space);
	}

	std::cout << end;

	context->gc_track_object(args);
	context->gc_track_object(kwargs);

	return nullptr;
}

BuiltinsModule::BuiltinsModule(ObjSpace* space, M_BaseObject* name) : Module(space, name)
{
	/* constants */
	add_def(std::string("None"), space->wrap_None());
	add_def(std::string("True"), space->wrap_True());
	add_def(std::string("False"), space->wrap_False());

	/* builtin type */
	add_def(std::string("bool"), space->bool_type());
	add_def(std::string("dict"), space->dict_type());
	add_def(std::string("int"), space->int_type());
	add_def(std::string("object"), space->object_type());
	add_def(std::string("str"), space->str_type());
	add_def(std::string("tuple"), space->tuple_type());
	add_def(std::string("type"), space->type_type());

	/* builtin exceptions */
#define ADD_EXCEPTION(name) add_def(std::string(#name), BaseException::get_bltin_exception_type(space, std::string(#name)))

	ADD_EXCEPTION(BaseException);
	ADD_EXCEPTION(Exception);
	ADD_EXCEPTION(TypeError);

	add_def(std::string("__import__"), new InterpFunctionWrapper(std::string("__import__"), builtin___import__, Signature(std::initializer_list<std::string>{"name", "globals", "locals", "from_list", "level"})));
	
	add_def(std::string("print"), new InterpFunctionWrapper(std::string("print"), builtin_print, Signature(std::string("args"), std::string("kwargs"))));
}
