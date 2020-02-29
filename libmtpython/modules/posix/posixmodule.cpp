#include <algorithm>

#include "modules/posix/posixmodule.h"
#include "interpreter/arguments.h"
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
#define STRUCT_STAT struct _stat
#define STAT _stat
#else
#define STRUCT_STAT struct stat
#define STAT stat
#endif

static std::vector<std::string> have_functions = {
    "HAVE_FSTATAT",
};

static M_BaseObject* os__exit(mtpython::vm::ThreadContext* context,
                              M_BaseObject* status)
{
    std::_Exit(context->get_space()->unwrap_int(status));
    return nullptr;
}

static M_BaseObject* os_listdir(mtpython::vm::ThreadContext* context,
                                const Arguments& args)
{
    static Signature listdir_signature({"path"});

    ObjSpace* space = context->get_space();
    std::vector<M_BaseObject*> scope;
    args.parse("listdir", nullptr, listdir_signature, scope,
               {space->wrap_None()});

    return space->wrap_None();
}

static M_BaseObject* os_stat(mtpython::vm::ThreadContext* context,
                             M_BaseObject* path, M_BaseObject* args,
                             M_BaseObject* kwargs)
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

PosixModule::PosixModule(ObjSpace* space, M_BaseObject* name)
    : BuiltinModule(space, name)
{
    std::vector<M_BaseObject*> _have_functions;
    std::for_each(have_functions.begin(), have_functions.end(),
                  [&_have_functions, space](const std::string& name) {
                      _have_functions.push_back(
                          space->wrap_str(space->current_thread(), name));
                  });

    add_def("_have_functions",
            space->new_list(space->current_thread(), _have_functions));

    add_def("_exit", new InterpFunctionWrapper("_exit", os__exit));
    add_def("listdir", new InterpFunctionWrapper("listdir", os_listdir));
    add_def("stat",
            new InterpFunctionWrapper(
                "stat", os_stat, Signature({"path"}, "args", "kwargs", {})));
}
