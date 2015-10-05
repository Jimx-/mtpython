#include "modules/_io/iomodule.h"
#include "modules/_io/textio.h"
#include "interpreter/gateway.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"
#include "interpreter/error.h"
#include "interpreter/pyframe.h"

using namespace mtpython::modules;
using namespace mtpython::objects;
using namespace mtpython::interpreter;

M_BaseObject* M_TextIOWrapper::__new__(mtpython::vm::ThreadContext* context, const Arguments& args)
{
	ObjSpace* space = context->get_space();
	M_BaseObject* instance = new M_TextIOWrapper(space);
	return space->wrap(instance);
}

M_BaseObject* M_TextIOWrapper::__init__(mtpython::vm::ThreadContext* context, const Arguments& args)
{
	static Signature init_signature({ "self", "buffer", "encoding", "errors", "newline", "line_buffering", "write_through" });
	ObjSpace* space = context->get_space();

	std::vector<M_BaseObject*> scope;
	args.parse("__init__", nullptr, init_signature, scope, { space->wrap_None(), space->wrap_None(), space->wrap_None(), space->wrap_None(), space->wrap_None() });
	
	M_BaseObject* self = scope[0];
	M_TextIOWrapper* as_tio = static_cast<M_TextIOWrapper*>(self);
	M_BaseObject* buffer = scope[1];
	M_BaseObject* wrapped_encoding = scope[2];
	as_tio->buffer = buffer;
	as_tio->encoding = wrapped_encoding;
	
	return space->wrap_None();
}

M_BaseObject* M_TextIOWrapper::__repr__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	std::string str;
	ObjSpace* space = context->get_space();
	str = "<_io.TextIOWrapper name=";
	M_BaseObject* name = space->findattr_str(self, "name");
	if (!name) return nullptr;
	M_BaseObject* name_repr = space->repr(name);
	str += space->unwrap_str(name_repr);

	str += " mode=";
	M_BaseObject* mode = space->findattr_str(self, "mode");
	if (!mode) return nullptr;
	M_BaseObject* mode_repr = space->repr(mode);
	str += space->unwrap_str(mode_repr);

	str += " encoding=";
	M_TextIOWrapper* as_tio = static_cast<M_TextIOWrapper*>(self);
	M_BaseObject* encoding_repr = space->repr(as_tio->encoding);
	str += space->unwrap_str(encoding_repr);

	str += ">";

	M_BaseObject* result = context->get_space()->wrap_str(str);

	context->delete_local_ref(name_repr);
	context->delete_local_ref(mode_repr);
	context->delete_local_ref(encoding_repr);

	return result;
}

M_BaseObject* M_TextIOWrapper::name_get(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	M_TextIOWrapper* as_tio = static_cast<M_TextIOWrapper*>(self);
	return context->get_space()->getattr_str(as_tio->buffer, "name");
}

M_BaseObject* M_TextIOWrapper::buffer_get(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	M_TextIOWrapper* as_tio = static_cast<M_TextIOWrapper*>(self);
	return as_tio->buffer;
}
