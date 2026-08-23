#include <fstream>

#include "utils/source_buffer.h"
#include "interpreter/compiler.h"
#include "interpreter/pycode.h"
#include "interpreter/module.h"
#include "objects/space_cache.h"
#include "objects/std/type_object.h"
#include "gc/garbage_collector_mmtk.h"
#include "interpreter/error.h"

#include "spdlog/spdlog.h"

using namespace mtpython::vm;
using namespace mtpython::objects;
using namespace mtpython::parse;
using namespace mtpython::interpreter;

PyVM::PyVM(ObjSpace* space, const std::string& executable)
    : main_thread_(this, space, true)
{
    this->space_ = space;

    gc_ = std::make_unique<gc::GarbageCollectorMmtk>(this);

    main_thread_.bind_gc(gc_.get());

    space->set_vm(this);

    init_bootstrap_path(executable);
}

Module* PyVM::init_main_module(ThreadContext* context)
{
    ObjSpace* space = context->get_space();
    M_BaseObject* main_name = space->wrap_str(context, "__main__");

    Module* module = new (context) Module(space, main_name);
    return module;
}

void PyVM::run_file(const std::string& filename)
{
    std::ifstream file;
    file.open(filename);

    if (!file) {
        throw mtpython::FileNotFoundException(
            ("unable to open file " + filename).c_str());
    }

    std::string source;
    file.seekg(0, std::ios::end);
    source.resize((unsigned int)file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&source[0], source.size());
    file.close();

    run_eval_string(&main_thread_, source, filename, false);
}

void PyVM::run_eval_string(ThreadContext* context, const std::string& source,
                           const std::string& filename, bool eval)
{
    std::string mode;
    if (eval)
        mode = "eval";
    else
        mode = "exec";

    ObjSpace* space = context->get_space();
    Module* main_module = init_main_module(context);
    M_BaseObject* globals = main_module->get_dict(space);

    space->setitem(globals, space->wrap_str(context, "__builtins__"),
                   space->get_builtin());
    if (filename != "")
        space->setitem(globals, space->wrap_str(context, "__file__"),
                       space->wrap_str(context, filename));

    compile_code(context, source, filename, mode)
        ->exec_code(context, globals, globals);
}

mtpython::interpreter::Code* PyVM::compile_code(ThreadContext* context,
                                                const std::string& source,
                                                const std::string& filename,
                                                const std::string& mode)
{
    ObjSpace* space = context->get_space();
    mtpython::interpreter::Module* mod =
        dynamic_cast<mtpython::interpreter::Module*>(space->get_builtin());

    M_BaseObject* code_obj =
        mod->call(context, "compile",
                  {space->wrap(context, source), space->wrap(context, filename),
                   space->wrap(context, mode), space->wrap(context, 0),
                   space->wrap(context, 0), space->wrap(context, 0)});

    mtpython::interpreter::PyCode* code =
        dynamic_cast<mtpython::interpreter::PyCode*>(code_obj);
    return code;
}

/* Format an uncaught app-level exception the way CPython's last
   traceback line does: "TypeError: message". The error value is not
   necessarily a string (e.g. StopIteration carries None, and to_string()
   is unimplemented for most objects), so stringification failures just
   drop the message instead of aborting the printer. */
static std::string
format_uncaught(InterpError& e, mtpython::objects::ObjSpace* space)
{
    std::string name = "Exception";
    auto* type_obj =
        dynamic_cast<mtpython::objects::M_StdTypeObject*>(e.get_type());
    if (type_obj) name = type_obj->get_name();

    std::string msg;
    try {
        msg = space->unwrap_str(e.get_value());
    } catch (...) {
    }

    if (msg.empty()) return name;
    return name + ": " + msg;
}

void PyVM::run_toplevel(std::function<void()> f)
{
    try {
        f();
    } catch (InterpError& e) {
        spdlog::error("{}", format_uncaught(e, space_));
    }
}

void PyVM::mark_roots() { space_->mark_roots(gc_.get()); }
