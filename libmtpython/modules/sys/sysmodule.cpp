#include "modules/sys/sysmodule.h"
#include "interpreter/gateway.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"
#include "interpreter/error.h"
#include "interpreter/pyframe.h"

using namespace mtpython::modules;
using namespace mtpython::objects;
using namespace mtpython::interpreter;

#ifdef _WIN32_
#define PLATFORM "win"
#elif defined(_LINUX_)
#define PLATFORM "linux2"
#endif

std::string SysModule::filesystem_encoding = "";

SysModule::SysModule(mtpython::vm::ThreadContext* context, M_BaseObject* name)
    : BuiltinModule(context->get_space(), name)
{
    ObjSpace* space = context->get_space();

    add_def("modules", space->new_dict(context));
    initstdio(context);
    initfsencoding(context);

    add_def("platform",
            space->wrap_str(vm::ThreadContext::current_thread(), PLATFORM));
    add_def("getfilesystemencoding",
            new InterpFunctionWrapper("getfilesystemencoding",
                                      SysModule::getfilesystemencoding));
}

static M_BaseObject* create_stdio(mtpython::vm::ThreadContext* context,
                                  M_BaseObject* io, int fd, bool writing,
                                  const std::string& name,
                                  const std::string& encoding,
                                  const std::string& errors, bool unbuffered)
{
    ObjSpace* space = context->get_space();
    int buffering = (writing && unbuffered) ? 0 : -1;
    std::string mode = writing ? "w" : "r";
    mode += "b";

    context->push_local_frame();

    M_BaseObject* open_impl = space->getattr_str(io, "open");
    if (!open_impl) return nullptr;

    M_BaseObject* buf = space->call_function(
        context, open_impl,
        {space->wrap_int(context, fd), space->wrap_str(context, mode),
         space->wrap_int(context, buffering), space->wrap_None(),
         space->wrap_None(), space->new_bool(false)});

    M_BaseObject* raw;
    if (buffering) {
        raw = space->getattr_str(buf, "raw");
        if (!raw) {
            return nullptr;
        }
    } else
        raw = buf;

    M_BaseObject* text = space->wrap_str(context, name);
    space->setattr(raw, space->wrap_str(context, "name"), text);

    M_BaseObject* newline;
#ifdef _WIN32_
    newline = space->wrap_None();
#else
    newline = space->wrap_str(context, "\n");
#endif

    bool line_buffering = false;

    M_BaseObject* wrapper_cls = space->getattr_str(io, "TextIOWrapper");
    if (!wrapper_cls) return nullptr;
    M_BaseObject* wrapper =
        space->call_function(context, wrapper_cls,
                             {buf, space->wrap_str(context, encoding),
                              space->wrap_str(context, errors), newline,
                              space->new_bool(line_buffering)});

    space->setattr(wrapper, space->wrap_str(context, "mode"),
                   space->wrap_str(context, mode));

    wrapper = context->pop_local_frame(wrapper);
    return wrapper;
}

void SysModule::initstdio(mtpython::vm::ThreadContext* context, bool unbuffered)
{
    ObjSpace* space = context->get_space();
    M_BaseObject* io = space->get__io();

    M_BaseObject* _stdin =
        create_stdio(context, io, 0, false, "<stdin>", "", "", unbuffered);
    add_def("stdin", _stdin);
    add_def("__stdin__", _stdin);

    M_BaseObject* _stdout =
        create_stdio(context, io, 1, true, "<stdout>", "", "", unbuffered);
    add_def("stdout", _stdout);
    add_def("__stdout__", _stdout);

    M_BaseObject* _stderr =
        create_stdio(context, io, 2, true, "<stderr>", "", "", unbuffered);
    add_def("stderr", _stderr);
    add_def("__stderr__", _stderr);
}

void SysModule::initfsencoding(mtpython::vm::ThreadContext* context)
{
    filesystem_encoding = "utf-8";
}

M_BaseObject*
SysModule::getfilesystemencoding(mtpython::vm::ThreadContext* context)
{
    return context->get_space()->wrap_str(context, filesystem_encoding);
}
