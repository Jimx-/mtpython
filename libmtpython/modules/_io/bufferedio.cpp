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

M_BaseObject* M_BufferedReader::__new__(mtpython::vm::ThreadContext* context, M_BaseObject* type, M_BaseObject* args, M_BaseObject* kwargs)
{
	ObjSpace* space = context->get_space();
	M_BaseObject* instance = new M_BufferedReader(space);
	return space->wrap(instance);
}

M_BaseObject* M_BufferedReader::__init__(mtpython::vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* args, M_BaseObject* kwargs)
{
	ObjSpace* space = context->get_space();
	std::vector<M_BaseObject*> scope;
	Arguments::parse_tuple_and_keywords(space, {"raw", "buffer_size"}, args, kwargs, scope, {space->wrap_int(DEFAULT_BUFFER_SIZE)});
	M_BaseObject* raw = scope[0];

	M_BufferedReader* as_br = static_cast<M_BufferedReader*>(self);
	as_br->raw = raw;
	space->setattr(as_br, space->wrap_str("raw"), raw);

	return nullptr;
}

M_BaseObject* M_BufferedWriter::__new__(mtpython::vm::ThreadContext* context, M_BaseObject* type, M_BaseObject* args, M_BaseObject* kwargs)
{
	ObjSpace* space = context->get_space();
	M_BaseObject* instance = new M_BufferedWriter(space);
	return space->wrap(instance);
}

M_BaseObject* M_BufferedWriter::__init__(mtpython::vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* args, M_BaseObject* kwargs)
{
	ObjSpace* space = context->get_space();
	std::vector<M_BaseObject*> scope;
	Arguments::parse_tuple_and_keywords(space, {"raw", "buffer_size"}, args, kwargs, scope, {space->wrap_int(DEFAULT_BUFFER_SIZE)});
	M_BaseObject* raw = scope[0];

	M_BufferedWriter* as_bw = static_cast<M_BufferedWriter*>(self);
	as_bw->raw = raw;
	space->setattr(as_bw, space->wrap_str("raw"), raw);

	return nullptr;
}
