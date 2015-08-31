#include "modules/builtins/bltinmodule.h"
#include "objects/bltin_exceptions.h"
#include "interpreter/gateway.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"

#include <iostream>

using namespace mtpython::modules;
using namespace mtpython::objects;
using namespace mtpython::interpreter;

static M_BaseObject* builtin___import__(mtpython::vm::ThreadContext* context, std::vector<M_BaseObject*>& args)
{
	return nullptr;

}

static M_BaseObject* builtin_compile(mtpython::vm::ThreadContext* context, std::vector<M_BaseObject*>& args)
{
	ObjSpace* space = context->get_space();

	std::string source = space->unwrap_str(args[0]);
	std::string filename = space->unwrap_str(args[1]);
	std::string mode = space->unwrap_str(args[2]);
	int flags  = space->unwrap_int(args[3]);

	M_BaseObject* code = context->get_compiler()->compile(source, filename, mode, flags);

	context->gc_track_object(args[0]);
	context->gc_track_object(args[1]);
	context->gc_track_object(args[2]);
	context->gc_track_object(args[3]);

	return space->wrap(code);
}

/* The famous print() */
static M_BaseObject* builtin_print(mtpython::vm::ThreadContext* context, M_BaseObject* args, M_BaseObject* kwargs)
{
	std::vector<M_BaseObject*> values;
	ObjSpace* space = context->get_space();

	M_BaseObject* wrapped_seq = space->getitem_str(kwargs, "seq");
	std::string seq = wrapped_seq ? space->unwrap_str(wrapped_seq) : " ";

	M_BaseObject* wrapped_end = space->getitem_str(kwargs, "end");
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
	add_def("None", space->wrap_None());
	add_def("True", space->wrap_True());
	add_def("False", space->wrap_False());

	/* builtin type */
	add_def("bool", space->bool_type());
	add_def("dict", space->dict_type());
	add_def("int", space->int_type());
	add_def("object", space->object_type());
	add_def("str", space->str_type());
	add_def("tuple", space->tuple_type());
	add_def("type", space->type_type());

	/* builtin exceptions */
#define ADD_EXCEPTION(name) add_def(#name, BaseException::get_bltin_exception_type(space, #name))

	ADD_EXCEPTION(BaseException);
	ADD_EXCEPTION(Exception);
	ADD_EXCEPTION(TypeError);
	ADD_EXCEPTION(StopIteration);

	add_def("__import__", new InterpFunctionWrapper("__import__", builtin___import__, Signature({"name", "globals", "locals", "from_list", "level"})));

	add_def("compile", new InterpFunctionWrapper("compile", builtin_compile, Signature({"source", "filename", "mode", "flags", "dont_inherit", "optimize"})));
	add_def("print", new InterpFunctionWrapper("print", builtin_print, Signature("args", "kwargs")));
}
