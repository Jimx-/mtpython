#include <stdexcept>
#include <string>
#include <fcntl.h>

#ifdef _WIN32_
#include <io.h>
#endif

#include "modules/_io/iomodule.h"
#include "modules/_io/fileio.h"
#include "interpreter/gateway.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"
#include "interpreter/error.h"
#include "interpreter/pyframe.h"

using namespace mtpython::modules;
using namespace mtpython::objects;
using namespace mtpython::interpreter;

M_BaseObject* M_FileIO::__new__(mtpython::vm::ThreadContext* context, const Arguments& args)
{
	ObjSpace* space = context->get_space();
	M_BaseObject* instance = new(context) M_FileIO(space);
	return space->wrap(context, instance);
}

static int decode_mode(const std::string& mode)
{
	int flags = 0;
	bool readable = false;
	bool writable = false;

	for (char c : mode) {
		switch (c) {
		case 'r':
			readable = true;
			break;
		case 'w':
			flags |= (O_CREAT | O_TRUNC);
			writable = true;
			break;
		}
	}

	if (readable && writable)
		flags |= O_RDWR;
	else if (readable)
		flags |= O_RDONLY;
	else
		flags |= O_WRONLY;

	return flags;
}

M_BaseObject* M_FileIO::__init__(mtpython::vm::ThreadContext* context, const Arguments& args)
{
	static Signature init_signature({ "self", "name", "mode", "closefd" });

	ObjSpace* space = context->get_space();
	
	std::vector<M_BaseObject*> scope;
	args.parse("__init__", nullptr, init_signature, scope, { space->wrap_str(context, "r"), space->new_bool(true) });
	M_BaseObject* self = scope[0];
	M_FileIO* as_fio = static_cast<M_FileIO*>(self);
	as_fio->name = nullptr;
	M_BaseObject* wrapped_name = scope[1];
	std::string name;
	M_BaseObject* wrapped_mode = scope[2];
	std::string mode = space->unwrap_str(wrapped_mode);
	M_BaseObject* wrapped_closefd = scope[3];
	bool closefd = space->is_true(wrapped_closefd);

	int fd = -1;
	try {
		fd = space->unwrap_int(wrapped_name);
		if (fd < 0) {
			throw InterpError(space->ValueError_type(), space->wrap_str(context, "negative file descriptor"));
		}
	} catch (const InterpError&) {
		name = space->unwrap_str(wrapped_name);
	}

	int flags = decode_mode(mode);

	if (fd > 0) {
		as_fio->fd = fd;
		as_fio->closefd = closefd;
	} else {
		as_fio->closefd = true;
		if (!closefd) {

		}

		int error = 0;
#ifdef _WIN32_
		error = _sopen_s(&fd, name.c_str(), flags, _SH_DENYRW, 0);
#else
		fd = open(name.c_str(), flags, 0666);
		if (fd < 0) error = -fd;
#endif

		if (error) {
			/* TODO: report error */
		}

		as_fio->fd = fd;
		space->setattr(as_fio, space->wrap_str(context, "name"), wrapped_name);
	}

	return nullptr;
}

M_BaseObject* M_FileIO::name_get(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
	M_FileIO* as_fio = static_cast<M_FileIO*>(self);
	return as_fio->name;
}

void M_FileIO::name_set(mtpython::vm::ThreadContext* context, M_BaseObject* obj, M_BaseObject* value)
{
	M_FileIO* as_fio = static_cast<M_FileIO*>(obj);
	as_fio->name = value;
}
