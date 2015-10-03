#include <stdexcept>
#include <string>
#include <fcntl.h>
#include <io.h>

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

M_BaseObject* M_FileIO::__new__(mtpython::vm::ThreadContext* context, M_BaseObject* type, M_BaseObject* args, M_BaseObject* kwargs)
{
	ObjSpace* space = context->get_space();
	M_BaseObject* instance = new M_FileIO(space);
	return space->wrap(instance);
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

M_BaseObject* M_FileIO::__init__(mtpython::vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* args, M_BaseObject* kwargs)
{
	ObjSpace* space = context->get_space();
	M_FileIO* as_fio = static_cast<M_FileIO*>(self);
	
	std::vector<M_BaseObject*> scope;
	Arguments::parse_tuple_and_keywords(space, {"name", "mode", "closefd"}, args, kwargs, scope, {space->wrap_str("r"), space->new_bool(true)});
	M_BaseObject* wrapped_name = scope[0];
	std::string name = space->unwrap_str(wrapped_name);
	M_BaseObject* wrapped_mode = scope[1];
	std::string mode = space->unwrap_str(wrapped_mode);
	M_BaseObject* wrapped_closefd = scope[2];
	bool closefd = space->is_true(wrapped_closefd);

	int fd = -1;
	try {
		fd = std::stoi(name, nullptr);
		if (fd < 0) {
			throw InterpError(space->ValueError_type(), space->wrap_str("negative file descriptor"));
		}
	} catch (const std::invalid_argument&) {
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
		space->setattr(as_fio, space->wrap_str("name"), wrapped_name);
	}

	context->delete_local_ref(args);
	context->delete_local_ref(kwargs);
	context->delete_local_ref(wrapped_name);
	context->delete_local_ref(wrapped_mode);
	context->delete_local_ref(wrapped_closefd);

	return nullptr;
}
