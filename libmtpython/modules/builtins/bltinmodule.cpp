#include <fstream>
#include <iostream>

#include "modules/builtins/bltinmodule.h"
#include "objects/bltin_exceptions.h"
#include "interpreter/gateway.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"
#include "interpreter/error.h"
#include "interpreter/pyframe.h"
#include "interpreter/module.h"
#include "utils/file_helper.h"

using namespace mtpython::modules;
using namespace mtpython::objects;
using namespace mtpython::interpreter;

typedef enum {
	UNKNOWN_MODTYPE, PY_SOURCE,
} ModType;

class ModuleSpec {
private:
	ModType type;
	std::string filename;
	std::string suffix;
public:
	ModuleSpec(ModType type, const std::string& filename, const std::string& suffix) : type(type), suffix(suffix), filename(filename) { }

	ModType get_type() { return type;  }
	std::string& get_filename() { return filename;  }
};

/* find the module named mod_name in sys.modules */
static M_BaseObject* check_sys_modules(ObjSpace* space, const std::string& mod_name)
{
	return space->finditem_str(space->get_sys()->get(space, "modules"), mod_name);
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

static ModType get_modtype(mtpython::vm::ThreadContext* context, const std::string& filepart,std::string& suffix)
{
	std::string py_file = filepart + ".py";
	if (mtpython::FileHelper::file_exists(py_file)) {
		suffix.assign(".py");
		return PY_SOURCE;
	}

	return UNKNOWN_MODTYPE;
}

static ModuleSpec* find_module(mtpython::vm::ThreadContext* context, const std::string& modulename, 
					M_BaseObject* w_modulename, const std::string& part, M_BaseObject* path)
{
	ObjSpace* space = context->get_space();
	if (!path) {	/* use sys.path */
		path = space->get_sys()->get(space, "path");
	}

	if (path) {
		std::vector<M_BaseObject*> path_items;
		path->unpack_iterable(space, path_items);
		for (auto wrapped_path : path_items) {
			std::string dir = space->unwrap_str(wrapped_path) + mtpython::FileHelper::sep;
			dir += part;

			std::string suffix;
			ModType mod_type = get_modtype(context, dir, suffix);
			if (mod_type == PY_SOURCE) {
				return new ModuleSpec(mod_type, dir + suffix, suffix);
			}
		}
	}

	return nullptr;
}

static void exec_code_module(mtpython::vm::ThreadContext* context, M_BaseObject* module, Code* code, const std::string& filename, bool set_path=true)
{
	ObjSpace* space = context->get_space();
	M_BaseObject* dict_name = space->wrap_str("__dict__");
	M_BaseObject* dict = space->getattr(module, dict_name);
	context->delete_local_ref(dict_name);

	if (set_path) {
		space->setitem(dict, space->wrap_str("__file__"), space->wrap_str(filename));
	}

	code->exec_code(context, dict, dict);
}

static M_BaseObject* load_source_module(mtpython::vm::ThreadContext* context, M_BaseObject* w_modulename, M_BaseObject* mod,
	const std::string& pathname, const std::string& source)
{
	Code* code_obj = context->get_compiler()->compile(source, pathname, "exec", 0);
	exec_code_module(context, mod, code_obj, pathname);

	return mod;
}

static M_BaseObject* load_module(mtpython::vm::ThreadContext* context, M_BaseObject* w_modulename, ModuleSpec* spec)
{
	ObjSpace* space = context->get_space();
	ModType mod_type = spec->get_type();
	if (mod_type == PY_SOURCE) {
		M_BaseObject* module = nullptr;
		try {
			module = space->getitem(space->get_sys()->get(space, "modules"), w_modulename);
		} catch (InterpError& exc) {
			if (!exc.match(space, space->KeyError_type())) throw exc;
		}
		if (!module) module = space->wrap(new Module(space, w_modulename));

		std::string filename = spec->get_filename();
		std::ifstream file;
		file.open(filename);
		std::string source;
		file.seekg(0, std::ios::end);
		source.resize((unsigned int)file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(&source[0], source.size());
		file.close();

		space->setitem(space->get_sys()->get(space, "modules"), w_modulename, module);
		space->setattr(module, space->wrap_str("__file__"), space->wrap_str(filename));
		module = load_source_module(context, w_modulename, module, filename, source);

		return module;
	}

	return nullptr;
}

static M_BaseObject* load_part(mtpython::vm::ThreadContext* context, M_BaseObject* path, const std::string& prefix, const std::string& part, M_BaseObject* parent)
{
	ObjSpace* space = context->get_space();
	std::string mod_name = prefix + part;
	M_BaseObject* w_mod_name = space->wrap_str(mod_name);
	M_BaseObject* mod = check_sys_modules(space, mod_name);

	if (mod && !space->i_is(mod, space->wrap_None())) {
		return mod;
	} else if (prefix.size() == 0 || path) {
		ModuleSpec* spec = find_module(context, mod_name, w_mod_name, part, path);

		if (spec) {
			M_BaseObject* mod = load_module(context, w_mod_name, spec);

			try {
				mod = space->getitem(space->get_sys()->get(space, "modules"), w_mod_name);
			} catch (InterpError& exc) {
				if (!exc.match(space, space->KeyError_type())) throw exc;
				else throw InterpError(space->ImportError_type(), w_mod_name);
			}

			if (parent) {
				space->setattr(parent, space->wrap_str(part), mod);
			}

			return mod;
		}
	}

	throw InterpError::format(space, space->ImportError_type(), "No module named %s", mod_name.c_str());
	return nullptr;
}

static M_BaseObject* _absolute_import(mtpython::vm::ThreadContext* context, const std::string& mod_name, int baselevel)
{
	std::size_t start = 0;
	M_BaseObject* mod = nullptr, *first = nullptr;
	M_BaseObject* path = nullptr;
	int level = 0;
	std::string prefix = "";
		
	while (true) {
		std::size_t dot = mod_name.find('.', start);
		std::size_t end;
		if (dot == std::string::npos) {
			end = mod_name.size();
		} else {
			end = dot;
		}

		std::string part = mod_name.substr(start, end);

		mod = load_part(context, path, prefix, part, mod);
		if (baselevel == level) first = mod;

		level++;

		start = end + 1;
		if (dot == std::string::npos) break;
	}
	return first;
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

	mod = absolute_import(context, mod_name, 0);
	return mod;
}

static M_BaseObject* builtin_abs(mtpython::vm::ThreadContext* context, M_BaseObject* obj)
{
	ObjSpace* space = context->get_space();
	M_BaseObject* result = space->abs(obj);

	if (!result) throw InterpError(space->TypeError_type(), space->wrap("bad operand type for abs()"));
	context->delete_local_ref(obj);

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

	context->delete_local_ref(args[0]);
	context->delete_local_ref(args[1]);
	context->delete_local_ref(args[2]);
	context->delete_local_ref(args[3]);

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
	context->delete_local_ref(obj);

	return result;
}

/* The famous print() */
static M_BaseObject* builtin_print(mtpython::vm::ThreadContext* context, M_BaseObject* args, M_BaseObject* kwargs)
{
	std::vector<M_BaseObject*> values;
	ObjSpace* space = context->get_space();

	M_BaseObject* wrapped_sep = space->finditem_str(kwargs, "sep");
	std::string sep = wrapped_sep ? space->unwrap_str(wrapped_sep) : " ";

	M_BaseObject* wrapped_end = space->finditem_str(kwargs, "end");
	std::string end = wrapped_end ? space->unwrap_str(wrapped_end) : "\n";

	space->unwrap_tuple(args, values);

	for (std::size_t i = 0; i < values.size(); i++) {
		if (i > 0) std::cout << sep;
		M_BaseObject* value = space->str(values[i]);
		std::cout << value->to_string(space);
		context->delete_local_ref(value);
	}

	std::cout << end;

	context->delete_local_ref(args);
	context->delete_local_ref(kwargs);

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
	add_def("set", space->set_type());
	add_def("str", space->str_type());
	add_def("tuple", space->tuple_type());
	add_def("list", space->list_type());
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
	ADD_EXCEPTION(KeyError);
	ADD_EXCEPTION(IndexError);

	add_def("__doc__", new InterpDocstringWrapper("Built-in functions, exceptions, and other objects.\n\nNoteworthy: None is the `nil' object; Ellipsis represents `...' in slices."));
	add_def("__import__", new InterpFunctionWrapper("__import__", builtin___import__, Signature({"name", "globals", "locals", "from_list", "level"})));

	add_def("abs", new InterpFunctionWrapper("abs", builtin_abs));
	add_def("compile", new InterpFunctionWrapper("compile", builtin_compile, Signature({"source", "filename", "mode", "flags", "dont_inherit", "optimize"})));
	add_def("globals", new InterpFunctionWrapper("globals", builtin_globals));
	add_def("len", new InterpFunctionWrapper("len", builtin_len));
	add_def("open", space->getattr_str(space->get__io(), "open"));
	add_def("print", new InterpFunctionWrapper("print", builtin_print, Signature("args", "kwargs")));
}
