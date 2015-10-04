#include "modules/posix/posixmodule.h"
#include "interpreter/gateway.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"
#include "interpreter/error.h"
#include "interpreter/pyframe.h"

using namespace mtpython::modules;
using namespace mtpython::objects;
using namespace mtpython::interpreter;

#include <sys/stat.h>
#ifdef _WIN32_
#define STRUCT_STAT		struct _stat
#define STAT			_stat
#else
#define STRUCT_STAT		struct stat
#define STAT			stat
#endif

static M_BaseObject* os_stat(mtpython::vm::ThreadContext* context, M_BaseObject* path, M_BaseObject* args, M_BaseObject* kwargs)
{
	STRUCT_STAT sbuf;

	ObjSpace* space = context->get_space();
	std::string pathname = space->unwrap_str(path);

	int result = STAT(pathname.c_str(), &sbuf);

	if (result) {
		/* TODO: report error */
	}

	return space->wrap_None();
}

PosixModule::PosixModule(ObjSpace* space, M_BaseObject* name) : BuiltinModule(space, name)
{
	add_def("stat", new InterpFunctionWrapper("stat", os_stat, Signature({"path"}, "args", "kwargs", {})));
}
