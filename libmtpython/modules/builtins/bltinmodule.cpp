#include "modules/builtins/bltinmodule.h"
#include "objects/bltin_exceptions.h"
#include "interpreter/gateway.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"
#include "interpreter/error.h"
#include "interpreter/pyframe.h"

#include <iostream>

using namespace mtpython::modules;
using namespace mtpython::objects;
using namespace mtpython::interpreter;

/* find the module named mod_name in sys.modules */
static M_BaseObject* check_sys_modules(ObjSpace* space, const std::string& mod_name)
{
	return space->getitem_str(space->get_sys()->get(space, "modules"),mod_name);
}

/* only look up the module in sys.modules without loading anything */
static M_BaseObject* lookup_sys_modules(ObjSpace* space, const std::string& mod_name)
{
	M_BaseObject* first_mod;
	if (mod_name.find('.') == std::string::npos) {
		first_mod = check_sys_modules(space, mod_name);
	}

	return first_mod;
}

static M_BaseObject* _absolute_import(mtpython::vm::ThreadContext* context, const std::string& mod_name, int level)
{
	return nullptr;
}

static M_BaseObject* absolute_import(mtpython::vm::ThreadContext* context, const std::string& mod_name, int level)
{
	ObjSpace* space = context->get_space();
	M_BaseObject* mod = nullptr;
	context->acquire_import_lock();

	mod = lookup_sys_modules(space, mod_name);
	if (mod && !space->i_is(mod, space->wrap_None())) goto out;

	mod = _absolute_import(context, mod_name, level);
out:
	context->release_import_lock();
	return mod;
}

static M_BaseObject* builtin___import__(mtpython::vm::ThreadContext* context, const std::vector<M_BaseObject*>& args)
{
	ObjSpace* space = context->get_space();
	std::string mod_name = space->unwrap_str(args[0]);
	int level = space->unwrap_int(args[4]);
	M_BaseObject* globals = args[1];
	M_BaseObject* mod;

	mod = absolute_import(context, mod_name, level);
	return mod;
}

static M_BaseObject* builtin_abs(mtpython::vm::ThreadContext* context, M_BaseObject* obj)
{
	ObjSpace* space = context->get_space();
	M_BaseObject* result = space->abs(obj);

	if (!result) throw InterpError(space->TypeError_type(), space->wrap("bad operand type for abs()"));
	context->gc_track_object(obj);

	return result;
}

static M_BaseObject* builtin_compile(mtpython::vm::ThreadContext* context, const std::vector<M_BaseObject*>& args)
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

static M_BaseObject* builtin_globals(mtpython::vm::ThreadContext* context)
{
	return context->top_frame()->get_globals();
}

static M_BaseObject* builtin_len(mtpython::vm::ThreadContext* context, M_BaseObject* obj)
{
	ObjSpace* space = context->get_space();
	M_BaseObject* result = space->len(obj);

	if (!result) throw InterpError(space->TypeError_type(), space->wrap("object has no len()"));
	context->gc_track_object(obj);

	return result;
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
		M_BaseObject* value = space->str(values[i]);
		std::cout << value->to_string(space);
	}

	std::cout << end;

	context->gc_track_object(args);
	context->gc_track_object(kwargs);

	return nullptr;
}

BuiltinsModule::BuiltinsModule(ObjSpace* space, M_BaseObject* name) : BuiltinModule(space, name)
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
	ADD_EXCEPTION(NameError);
	ADD_EXCEPTION(UnboundLocalError);
	ADD_EXCEPTION(AttributeError);
	ADD_EXCEPTION(ImportError);
	ADD_EXCEPTION(ValueError);
	ADD_EXCEPTION(SystemError);

	add_def("__doc__", new InterpDocstringWrapper("Built-in functions, exceptions, and other objects.\n\nNoteworthy: None is the `nil' object; Ellipsis represents `...' in slices."));
	add_def("__import__", new InterpFunctionWrapper("__import__", builtin___import__, Signature({"name", "globals", "locals", "from_list", "level"})));

	add_def("abs", new InterpFunctionWrapper("abs", builtin_abs));
	add_def("compile", new InterpFunctionWrapper("compile", builtin_compile, Signature({"source", "filename", "mode", "flags", "dont_inherit", "optimize"})));
	add_def("globals", new InterpFunctionWrapper("globals", builtin_globals));
	add_def("len", new InterpFunctionWrapper("len", builtin_len));
	add_def("print", new InterpFunctionWrapper("print", builtin_print, Signature("args", "kwargs")));
}
