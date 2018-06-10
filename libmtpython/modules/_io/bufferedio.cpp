#include "modules/_io/iomodule.h"
#include "modules/_io/bufferedio.h"
#include "interpreter/gateway.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"
#include "interpreter/error.h"
#include "interpreter/pyframe.h"

using namespace mtpython::modules;
using namespace mtpython::objects;
using namespace mtpython::interpreter;

M_BaseObject* BufferedBase::name_get(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	BufferedBase* bb = static_cast<BufferedBase*>(self);
	ObjSpace* space = context->get_space();
	return space->getattr_str(bb->raw, "name");
}

M_BaseObject* M_BufferedReader::__new__(mtpython::vm::ThreadContext* context, const Arguments& args)
{
	ObjSpace* space = context->get_space();
	M_BaseObject* instance = new(context) M_BufferedReader(space);
	return space->wrap(context, instance);
}

M_BaseObject* M_BufferedReader::__init__(mtpython::vm::ThreadContext* context, const Arguments& args)
{
	static Signature init_signature({ "self", "raw", "buffer_size" });

	ObjSpace* space = context->get_space();
	std::vector<M_BaseObject*> scope;
	args.parse("__init__", nullptr, init_signature, scope, { space->wrap_int(context, DEFAULT_BUFFER_SIZE) });
	M_BaseObject* self = scope[0];
	M_BaseObject* raw = scope[1];

	M_BufferedReader* as_br = static_cast<M_BufferedReader*>(self);
	as_br->raw = raw;
	space->setattr(as_br, space->wrap_str(context, "raw"), raw);

	return nullptr;
}

M_BaseObject* M_BufferedWriter::__new__(mtpython::vm::ThreadContext* context, const Arguments& args)
{
	ObjSpace* space = context->get_space();
	M_BaseObject* instance = new(context) M_BufferedWriter(space);
	return space->wrap(context, instance);
}

M_BaseObject* M_BufferedWriter::__init__(mtpython::vm::ThreadContext* context, const Arguments& args)
{
	static Signature init_signature({ "self", "raw", "buffer_size" });

	ObjSpace* space = context->get_space();
	std::vector<M_BaseObject*> scope;
	args.parse("__init__", nullptr, init_signature, scope, { space->wrap_int(context, DEFAULT_BUFFER_SIZE) });

	M_BaseObject* self = scope[0];
	M_BaseObject* raw = scope[1];

	M_BufferedWriter* as_bw = static_cast<M_BufferedWriter*>(self);
	as_bw->raw = raw;
	space->setattr(as_bw, space->wrap_str(context, "raw"), raw);

	return nullptr;
}
