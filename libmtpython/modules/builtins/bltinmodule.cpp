#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>

#include "modules/builtins/bltinmodule.h"
#include "objects/bltin_exceptions.h"
#include "interpreter/gateway.h"
#include "interpreter/function.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"
#include "interpreter/error.h"
#include "interpreter/pyframe.h"
#include "utils/file_helper.h"
#include "gc/garbage_collector.h"

#include "spdlog/spdlog.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

namespace fs = std::filesystem;

namespace mtpython {

namespace modules {

enum class ModType {
    UNKNOWN,
    PY_SOURCE,
    PKG_DIRECTORY,
};

class ModuleSpec {
private:
    ModType type;
    fs::path filename;
    std::string suffix;

public:
    ModuleSpec(ModType type, const fs::path& filename,
               const std::string& suffix)
        : type(type), suffix(suffix), filename(filename)
    {}

    ModType get_type() { return type; }
    fs::path& get_filename() { return filename; }
};

/* find the module named mod_name in sys.modules */
static M_BaseObject* check_sys_modules(ObjSpace* space,
                                       const std::string& mod_name)
{
    return space->finditem_str(space->get_sys()->get(space, "modules"),
                               mod_name);
}

/* only look up the module in sys.modules without loading anything */
static M_BaseObject* lookup_sys_modules(ObjSpace* space,
                                        const std::string& mod_name)
{
    M_BaseObject* first_mod = nullptr;
    // if (mod_name.find('.') == std::string::npos) {
    first_mod = check_sys_modules(space, mod_name);
    //}

    return first_mod;
}

static ModType get_modtype(const fs::path& filepart, std::string& suffix)
{
    fs::path py_file = filepart;

    py_file.replace_extension(".py");
    if (fs::is_regular_file(py_file)) {
        suffix.assign(".py");
        return ModType::PY_SOURCE;
    }

    return ModType::UNKNOWN;
}

static bool has_init_module(const fs::path& filepart)
{
    auto init_path = filepart / "__init__";

    init_path.replace_extension(".py");
    return fs::is_regular_file(init_path);
}

static ModuleSpec* find_module(mtpython::vm::ThreadContext* context,
                               const std::string& modulename,
                               M_BaseObject* w_modulename,
                               const std::string& part, M_BaseObject* path)
{
    ObjSpace* space = context->get_space();
    if (!path) { /* use sys.path */
        path = space->get_sys()->get(space, "path");
    }

    if (path) {
        std::vector<M_BaseObject*> path_items;
        path->unpack_iterable(space, path_items);
        for (auto wrapped_path : path_items) {
            auto dir = fs::path(space->unwrap_str(wrapped_path)) / part;

            if (fs::is_directory(dir)) {
                if (has_init_module(dir)) {
                    return new ModuleSpec(ModType::PKG_DIRECTORY, dir, "");
                }
            }

            std::string suffix;
            ModType mod_type = get_modtype(dir, suffix);
            if (mod_type == ModType::PY_SOURCE) {
                dir.replace_extension(suffix);
                return new ModuleSpec(mod_type, dir, suffix);
            }
        }
    }

    return nullptr;
}

static void exec_code_module(mtpython::vm::ThreadContext* context,
                             M_BaseObject* module, Code* code,
                             const std::string& filename, bool set_path = true)
{
    ObjSpace* space = context->get_space();
    M_BaseObject* dict_name = space->wrap_str(context, "__dict__");
    M_BaseObject* dict = space->getattr(module, dict_name);

    if (set_path) {
        space->setitem(dict, space->wrap_str(context, "__file__"),
                       space->wrap_str(context, filename));
    }

    code->exec_code(context, dict, dict);
}

static M_BaseObject* load_source_module(mtpython::vm::ThreadContext* context,
                                        M_BaseObject* w_modulename,
                                        M_BaseObject* mod,
                                        const std::string& pathname,
                                        const std::string& source)
{
    Code* code_obj =
        context->get_compiler()->compile(source, pathname, "exec", 0);
    exec_code_module(context, mod, code_obj, pathname);

    return mod;
}

static M_BaseObject* load_module(mtpython::vm::ThreadContext* context,
                                 M_BaseObject* w_modulename, ModuleSpec* spec)
{
    ObjSpace* space = context->get_space();
    ModType mod_type = spec->get_type();

    spdlog::debug("Loading module `{}'", std::string(spec->get_filename()));

    switch (mod_type) {
    case ModType::PY_SOURCE:
    case ModType::PKG_DIRECTORY: {
        M_BaseObject* module = nullptr;
        try {
            module = space->getitem(space->get_sys()->get(space, "modules"),
                                    w_modulename);
        } catch (InterpError& exc) {
            if (!exc.match(space, space->KeyError_type())) throw exc;
        }
        if (!module)
            module =
                space->wrap(context, new (context) Module(space, w_modulename));

        space->setitem(space->get_sys()->get(space, "modules"), w_modulename,
                       module);
        space->setattr(module, space->wrap_str(context, "__file__"),
                       space->wrap_str(context, spec->get_filename()));

        if (mod_type == ModType::PY_SOURCE) {
            std::string filename = spec->get_filename();
            std::ifstream file;
            file.open(filename);
            std::string source;
            file.seekg(0, std::ios::end);
            source.resize((unsigned int)file.tellg());
            file.seekg(0, std::ios::beg);
            file.read(&source[0], source.size());
            file.close();

            module = load_source_module(context, w_modulename, module, filename,
                                        source);

            return module;
        } else if (mod_type == ModType::PKG_DIRECTORY) {
            M_BaseObject* w_new_path = space->new_list(
                context, {space->wrap_str(context, spec->get_filename())});
            space->setattr(module, space->wrap_str(context, "__path__"),
                           w_new_path);
            std::unique_ptr<ModuleSpec> new_spec;
            new_spec.reset(find_module(context, "__init__", nullptr, "__init__",
                                       w_new_path));

            if (!new_spec) return module;

            module = load_module(context, w_modulename, new_spec.get());

            return module;
        }
        break;
    }

    default:
        break;
    }

    return nullptr;
}

static M_BaseObject* load_part(mtpython::vm::ThreadContext* context,
                               M_BaseObject* path, const std::string& prefix,
                               const std::string& part, M_BaseObject* parent)
{
    ObjSpace* space = context->get_space();
    std::string mod_name = prefix + part;
    M_BaseObject* w_mod_name = space->wrap_str(context, mod_name);
    M_BaseObject* mod = check_sys_modules(space, mod_name);

    if (mod && !space->i_is(mod, space->wrap_None())) {
        return mod;
    } else if (prefix.size() == 0 || path) {
        std::unique_ptr<ModuleSpec> spec;
        spec.reset(find_module(context, mod_name, w_mod_name, part, path));

        if (spec) {
            M_BaseObject* mod = load_module(context, w_mod_name, spec.get());

            try {
                mod = space->getitem(space->get_sys()->get(space, "modules"),
                                     w_mod_name);
            } catch (InterpError& exc) {
                if (!exc.match(space, space->KeyError_type()))
                    throw exc;
                else
                    throw InterpError(space->ImportError_type(), w_mod_name);
            }

            if (parent) {
                space->setattr(parent, space->wrap_str(context, part), mod);
            }

            return mod;
        }
    }

    throw InterpError::format(space, space->ImportError_type(),
                              "No module named %s", mod_name.c_str());
    return nullptr;
}

static M_BaseObject* _absolute_import(mtpython::vm::ThreadContext* context,
                                      const std::string& mod_name,
                                      int baselevel)
{
    std::size_t start = 0;
    M_BaseObject *mod = nullptr, *first = nullptr;
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

static M_BaseObject* absolute_import(mtpython::vm::ThreadContext* context,
                                     const std::string& mod_name, int level)
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

static M_BaseObject* builtin___import__(mtpython::vm::ThreadContext* context,
                                        const std::vector<M_BaseObject*>& args)
{
    ObjSpace* space = context->get_space();
    std::string mod_name = space->unwrap_str(args[0]);
    int level = space->unwrap_int(args[4]);
    M_BaseObject* globals = args[1];
    M_BaseObject* mod;

    mod = absolute_import(context, mod_name, 0);
    return mod;
}

static M_BaseObject*
builtin___build_class__(mtpython::vm::ThreadContext* context,
                        M_BaseObject* func, M_BaseObject* name,
                        M_BaseObject* bases, M_BaseObject* kwargs)
{
    ObjSpace* space = context->get_space();

    std::vector<M_BaseObject*> bases_w;
    space->unwrap_tuple(bases, bases_w);

    bool isclass = false;
    M_BaseObject* metaclass = space->finditem_str(kwargs, "metaclass");
    if (metaclass) {
        isclass = true;
    } else {
        if (bases_w.size() > 0) {
            metaclass = space->type(bases_w[0]);
        } else {
            metaclass = space->get_type_by_name("type");
        }
    }

    M_BaseObject *prep, *ns;
    try {
        prep = space->getattr_str(metaclass, "__prepare__");

        ns = space->call_function(context, prep, {name, bases});
    } catch (InterpError& exc) {
        if (!exc.match(space, space->AttributeError_type())) throw exc;

        ns = space->new_dict(context);
    }

    Function* func_obj = dynamic_cast<Function*>(func);
    if (!func_obj) {
        throw InterpError(
            space->TypeError_type(),
            space->wrap_str(context,
                            "__build_class__: func must be a function"));
    }

    M_BaseObject* __class__cell = func_obj->get_code()->exec_code(
        context, func_obj->get_globals(), ns, func_obj);

    M_BaseObject* class_obj =
        space->call_function(context, metaclass, {name, bases, ns});

    Cell* cell = dynamic_cast<Cell*>(__class__cell);
    if (cell) {
        cell->set(class_obj);
    }

    return class_obj;
}

static M_BaseObject* builtin_abs(mtpython::vm::ThreadContext* context,
                                 M_BaseObject* obj)
{
    ObjSpace* space = context->get_space();
    M_BaseObject* result = space->abs(obj);

    if (!result)
        throw InterpError(space->TypeError_type(),
                          space->wrap(context, "bad operand type for abs()"));

    return result;
}

static M_BaseObject* builtin_compile(mtpython::vm::ThreadContext* context,
                                     const std::vector<M_BaseObject*>& args)
{
    ObjSpace* space = context->get_space();

    std::string source = space->unwrap_str(args[0]);
    std::string filename = space->unwrap_str(args[1]);
    std::string mode = space->unwrap_str(args[2]);
    int flags = space->unwrap_int(args[3]);

    M_BaseObject* code =
        context->get_compiler()->compile(source, filename, mode, flags);

    return space->wrap(context, code);
}

static M_BaseObject* builtin_getattr(mtpython::vm::ThreadContext* context,
                                     const Arguments& args)
{
    static Signature getattr_signature({"obj", "name", "defval"});

    ObjSpace* space = context->get_space();
    std::vector<M_BaseObject*> scope;
    args.parse("getattr", nullptr, getattr_signature, scope, {nullptr});

    M_BaseObject* obj = scope[0];
    M_BaseObject* name = scope[1];
    M_BaseObject* defval = scope[2];

    try {
        return space->getattr(obj, name);
    } catch (InterpError& e) {
        if (defval) {
            if (e.match(space, space->AttributeError_type())) {
                return defval;
            }
        }
        throw;
    }

    return nullptr;
}

static M_BaseObject* builtin_globals(mtpython::vm::ThreadContext* context)
{
    return context->top_frame()->get_globals();
}

/* isinstance() & issubtype() */
static M_BaseObject* _get_class(ObjSpace* space, M_BaseObject* obj)
{
    try {
        return space->getattr_str(obj, "__class__");
    } catch (InterpError& e) {
        if (!e.match(space, space->AttributeError_type())) {
            throw;
        }
    }
    return space->type(obj);
}

static bool _isinstance(ObjSpace* space, M_BaseObject* obj, M_BaseObject* cls,
                        bool override = false)
{
    if (space->i_isinstance(cls, space->get_type_by_name("tuple"))) {
        std::vector<M_BaseObject*> clsv;
        space->unwrap_tuple(cls, clsv);

        for (auto type : clsv) {
            if (_isinstance(space, obj, type)) return true;
        }
        return false;
    }

    M_BaseObject* result;
    try {
        if (override) {
            result = space->isinstance_override(obj, cls);
        } else {
            result = space->isinstance(obj, cls);
        }
    } catch (InterpError& e) {
        if (!e.match(space, space->TypeError_type())) {
            throw;
        }
    }

    if (space->is_true(result)) return true;
    M_BaseObject* klass = _get_class(space, obj);

    try {
        if (space->i_is(klass, space->type(obj))) return false;

        if (override) {
            result = space->issubtype_override(klass, cls);
        } else {
            result = space->issubtype(klass, cls);
        }

        return space->is_true(result);
    } catch (InterpError&) {
        return false;
    }

    return false;
}

static bool _issubtype(ObjSpace* space, M_BaseObject* sub, M_BaseObject* cls,
                       bool override = false)
{
    if (space->i_isinstance(cls, space->get_type_by_name("tuple"))) {
        std::vector<M_BaseObject*> clsv;
        space->unwrap_tuple(cls, clsv);

        for (auto type : clsv) {
            if (_issubtype(space, sub, type)) return true;
        }
        return false;
    }

    M_BaseObject* result;
    try {
        if (override) {
            result = space->issubtype_override(sub, cls);
        } else {
            result = space->issubtype(sub, cls);
        }

        return space->is_true(result);
    } catch (InterpError& e) {
        if (!e.match(space, space->TypeError_type())) {
            throw;
        }
    }

    return false;
}

static M_BaseObject* builtin_isinstance(mtpython::vm::ThreadContext* context,
                                        M_BaseObject* obj, M_BaseObject* cls)
{
    ObjSpace* space = context->get_space();
    return space->new_bool(_isinstance(space, obj, cls, true));
}

static M_BaseObject* builtin_issubclass(mtpython::vm::ThreadContext* context,
                                        M_BaseObject* sub, M_BaseObject* cls)
{
    ObjSpace* space = context->get_space();
    return space->new_bool(_issubtype(space, sub, cls, true));
}

static M_BaseObject* builtin_iter(mtpython::vm::ThreadContext* context,
                                  const Arguments& args)
{
    static Signature iter_signature({"obj", "sentinel"});

    ObjSpace* space = context->get_space();
    std::vector<M_BaseObject*> scope;
    args.parse("iter", nullptr, iter_signature, scope, {space->wrap_None()});

    M_BaseObject* obj = scope[0];
    M_BaseObject* sentinel = scope[1];

    if (space->i_is(sentinel, space->wrap_None())) {
        return context->get_space()->iter(obj);
    }

    return space->wrap_None();
}

static M_BaseObject* builtin_len(mtpython::vm::ThreadContext* context,
                                 M_BaseObject* obj)
{
    ObjSpace* space = context->get_space();
    M_BaseObject* result = space->len(obj);

    if (!result)
        throw InterpError(space->TypeError_type(),
                          space->wrap(context, "object has no len()"));

    return result;
}

/* The famous print() */
static M_BaseObject* builtin_print(mtpython::vm::ThreadContext* context,
                                   M_BaseObject* args, M_BaseObject* kwargs)
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
    }

    std::cout << end;

    return nullptr;
}

/*
 * Range
 */
class M_RangeIter : public M_BaseObject {
private:
    ObjSpace* space;
    int current;
    int remaining;
    int step;

public:
    M_RangeIter(ObjSpace* space, int current, int remaining, int step)
        : space(space), current(current), remaining(remaining), step(step)
    {}

    Typedef* get_typedef();

    static M_BaseObject* __next__(mtpython::vm::ThreadContext* context,
                                  M_BaseObject* self)
    {
        ObjSpace* space = context->get_space();

        M_RangeIter* iter = static_cast<M_RangeIter*>(self);
        if (iter->remaining > 0) {
            iter->lock();
            int index = iter->current;
            iter->current = index + iter->step;
            iter->remaining--;
            iter->unlock();
            return space->wrap_int(context, index);
        }

        throw InterpError(space->StopIteration_type(), space->wrap_None());
    }
};

Typedef* M_RangeIter::get_typedef()
{
    static Typedef range_iterator_typedef(
        "range_iterator",
        {
            {"__next__",
             new InterpFunctionWrapper("__next__", M_RangeIter::__next__)},
        });

    return &range_iterator_typedef;
}

class M_Range : public M_BaseObject {
private:
    M_BaseObject* start;
    M_BaseObject* stop;
    M_BaseObject* step;
    M_BaseObject* length;

public:
    M_Range(M_BaseObject* start, M_BaseObject* stop, M_BaseObject* step,
            M_BaseObject* length)
        : start(start), stop(stop), step(step), length(length)
    {}

    virtual void mark_children(gc::GarbageCollector* gc)
    {
        gc->mark_object(start);
        gc->mark_object(stop);
        gc->mark_object(step);
        gc->mark_object(length);
    }

    static M_BaseObject* __new__(mtpython::vm::ThreadContext* context,
                                 const Arguments& args)
    {
        static Signature new_signature({"type", "start", "stop", "step"});

        ObjSpace* space = context->get_space();
        M_BaseObject* None = space->wrap_None();
        std::vector<M_BaseObject*> scope;
        args.parse("__new__", nullptr, new_signature, scope, {None, None});

        M_BaseObject* start = scope[1];
        M_BaseObject* stop = scope[2];
        M_BaseObject* step = scope[3];

        if (space->i_is(stop, None)) {
            if (!space->i_is(step, None)) {
                throw InterpError::format(
                    space, space->TypeError_type(),
                    "'%s' object cannot be interpreted as an integer",
                    space->get_type_name(stop).c_str());
            }

            stop = start;
            start = space->wrap_int(context, 0);
        }

        if (space->i_is(step, None)) {
            step = space->wrap_int(context, 1);
        }

        if (space->unwrap_int(step) == 0) {
            throw InterpError(
                space->ValueError_type(),
                space->wrap_str(context, "step argument must not be zero"));
        }

        int i_start = space->unwrap_int(start);
        int i_stop = space->unwrap_int(stop);
        int i_step = space->unwrap_int(step);
        int i_length = 0;

        if (i_step < 0) {
            i_step = -i_step;
            int tmp = i_start;
            i_start = i_stop;
            i_stop = tmp;
        }

        if (i_start < i_stop) {
            int diff = i_stop - i_start - 1;
            i_length = diff / i_step + 1;
        }

        M_BaseObject* range = new (context)
            M_Range(start, stop, step, space->wrap_int(context, i_length));

        return space->wrap(context, range);
    }

    static M_BaseObject* __iter__(mtpython::vm::ThreadContext* context,
                                  M_BaseObject* self)
    {
        ObjSpace* space = context->get_space();

        M_Range* range = static_cast<M_Range*>(self);
        int start = space->unwrap_int(range->start);
        int stop = space->unwrap_int(range->stop);
        int length = space->unwrap_int(range->length);
        int step = space->unwrap_int(range->step);

        M_RangeIter* iter =
            new (context) M_RangeIter(space, start, length, step);

        return iter;
    }

    static M_BaseObject* __getitem__(mtpython::vm::ThreadContext* context,
                                     M_BaseObject* self, M_BaseObject* index)
    {
        ObjSpace* space = context->get_space();

        M_Range* range = static_cast<M_Range*>(self);
        int start = space->unwrap_int(range->start);
        int stop = space->unwrap_int(range->stop);
        int length = space->unwrap_int(range->length);
        int step = space->unwrap_int(range->step);

        int i = space->i_get_index(index, space->IndexError_type(),
                                   space->wrap_str(context, "range index"));
        if (i < 0) {
            i += length;
        }

        if (i >= length || i < 0) {
            throw InterpError(
                space->IndexError_type(),
                space->wrap_str(context, "range object index out of range"));
        }

        return space->wrap_int(context, start + i * step);
    }

    static Typedef* _range_typedef()
    {
        static Typedef Range_typedef(
            "range",
            {
                {"__new__",
                 new InterpFunctionWrapper("__new__", M_Range::__new__)},
                {"__iter__",
                 new InterpFunctionWrapper("__iter__", M_Range::__iter__)},
                {"__getitem__", new InterpFunctionWrapper(
                                    "__getitem__", M_Range::__getitem__)},
            });

        return &Range_typedef;
    }

    Typedef* get_typedef() { return _range_typedef(); }
};

/*
 * Reversed iterator
 */
class M_ReversedIterObject : public M_BaseObject {
private:
    int remaining;
    M_BaseObject* obj;

public:
    M_ReversedIterObject(mtpython::vm::ThreadContext* context,
                         M_BaseObject* obj)
        : obj(obj)
    {
        ObjSpace* space = context->get_space();
        remaining = space->unwrap_int(space->len(obj)) - 1;
        if (!space->lookup(obj, "__getitem__")) {
            throw InterpError(
                space->TypeError_type(),
                space->wrap_str(context,
                                "reversed() argument must be a sequence"));
        }
    }

    Typedef* get_typedef();

    virtual void mark_children(gc::GarbageCollector* gc)
    {
        gc->mark_object(obj);
    }

    static M_BaseObject* __next__(mtpython::vm::ThreadContext* context,
                                  M_BaseObject* self)
    {
        ObjSpace* space = context->get_space();

        M_ReversedIterObject* iter = static_cast<M_ReversedIterObject*>(self);
        iter->lock();

        if (iter->remaining >= 0) {
            M_BaseObject* item;
            try {
                item = space->getitem(
                    iter->obj, space->wrap_int(context, iter->remaining));
                iter->remaining--;

                iter->unlock();
                return item;
            } catch (InterpError& exc) {
                iter->unlock();
                iter->obj = nullptr;
                if (!exc.match(space, space->IndexError_type())) throw exc;
                throw InterpError(space->StopIteration_type(),
                                  space->wrap_None());
            }
        }

        iter->remaining = -1;
        iter->unlock();

        throw InterpError(space->StopIteration_type(), space->wrap_None());
    }

    static M_BaseObject* __iter__(mtpython::vm::ThreadContext* context,
                                  M_BaseObject* self)
    {
        ObjSpace* space = context->get_space();
        return space->wrap(context, self);
    }
};

Typedef* M_ReversedIterObject::get_typedef()
{
    static Typedef reversed_iter_typedef(
        "reverseiterator",
        {
            {"__next__", new InterpFunctionWrapper(
                             "__next__", M_ReversedIterObject::__next__)},
            {"__iter__", new InterpFunctionWrapper(
                             "__iter__", M_ReversedIterObject::__iter__)},
        });

    return &reversed_iter_typedef;
}

static M_BaseObject* builtin_reversed(mtpython::vm::ThreadContext* context,
                                      M_BaseObject* seq)
{
    ObjSpace* space = context->get_space();

    M_BaseObject* reversed = space->lookup(seq, "__reversed__");
    if (reversed) {
        M_BaseObject* reversed_impl = space->get(reversed, seq);
        return space->call_function(context, reversed_impl, {});
    }

    return new (context) M_ReversedIterObject(context, seq);
}

/*
 *  Property
 */
class M_Property : public M_BaseObject {
private:
    M_BaseObject* fget;
    M_BaseObject* fset;
    M_BaseObject* fdel;
    M_BaseObject* doc;

public:
    M_Property(ObjSpace* space, M_BaseObject* fget = nullptr,
               M_BaseObject* fset = nullptr, M_BaseObject* fdel = nullptr,
               M_BaseObject* doc = nullptr)
    {
        this->fget = fget ? fget : space->wrap_None();
        this->fset = fset ? fset : space->wrap_None();
        this->fdel = fdel ? fdel : space->wrap_None();
        this->doc = doc ? doc : space->wrap_None();
    }

    virtual void mark_children(gc::GarbageCollector* gc)
    {
        gc->mark_object(fget);
        gc->mark_object(fset);
        gc->mark_object(fdel);
        gc->mark_object(doc);
    }

    static M_BaseObject* __new__(mtpython::vm::ThreadContext* context,
                                 const Arguments& args)
    {
        static Signature new_signature(
            {"type", "getter", "setter", "deleter", "doc"});
        ObjSpace* space = context->get_space();

        std::vector<M_BaseObject*> scope;
        args.parse("__new__", nullptr, new_signature, scope,
                   {space->wrap_None(), space->wrap_None(), space->wrap_None(),
                    space->wrap_None()});

        M_Property* instance = new (context)
            M_Property(space, scope[1], scope[2], scope[3], scope[4]);
        return space->wrap(context, instance);
    }

    static M_BaseObject* __get__(mtpython::vm::ThreadContext* context,
                                 const Arguments& args)
    {
        static Signature get_signature({"self", "obj", "type"});
        ObjSpace* space = context->get_space();

        std::vector<M_BaseObject*> scope;
        args.parse("__get__", nullptr, get_signature, scope,
                   {space->wrap_None()});

        M_Property* self = static_cast<M_Property*>(scope[0]);
        M_BaseObject* obj = scope[1];
        M_BaseObject* cls = scope[2];

        if (space->i_is(obj, space->wrap_None())) {
            return space->wrap(context, self);
        }

        if (space->i_is(self->fget, space->wrap_None())) {
            throw InterpError(space->TypeError_type(),
                              space->wrap_str(context, "unreadable attribute"));
        }

        return space->call_function(context, self->fget, {obj});
    }

    static M_BaseObject* __set__(mtpython::vm::ThreadContext* context,
                                 M_BaseObject* _self, M_BaseObject* obj,
                                 M_BaseObject* value)
    {
        M_Property* self = static_cast<M_Property*>(_self);
        ObjSpace* space = context->get_space();
        if (space->i_is(self->fset, space->wrap_None())) {
            throw InterpError(space->TypeError_type(),
                              space->wrap_str(context, "can't set attribute"));
        }
        space->call_function(context, self->fset, {obj, value});
        return space->wrap_None();
    }

    static M_BaseObject* setter(mtpython::vm::ThreadContext* context,
                                M_BaseObject* _self, M_BaseObject* _setter)
    {
        return copy(context, _self, nullptr, _setter, nullptr);
    }

    static M_BaseObject* copy(mtpython::vm::ThreadContext* context,
                              M_BaseObject* _self, M_BaseObject* _getter,
                              M_BaseObject* _setter, M_BaseObject* _deleter)
    {
        ObjSpace* space = context->get_space();
        M_Property* self = static_cast<M_Property*>(_self);
        M_BaseObject* getter = _getter ? _getter : self->fget;
        M_BaseObject* setter = _setter ? _setter : self->fset;
        M_BaseObject* deleter = _deleter ? _deleter : self->fdel;
        M_BaseObject* doc = self->doc;

        M_BaseObject* type = self->get_class(space);
        return space->call_function(context, type,
                                    {getter, setter, deleter, doc});
    }

    static Typedef* _property_typedef()
    {
        static Typedef Property_typedef(
            "property",
            {
                {"__new__",
                 new InterpFunctionWrapper("__new__", M_Property::__new__)},
                {"__get__",
                 new InterpFunctionWrapper("__get__", M_Property::__get__)},
                {"__set__",
                 new InterpFunctionWrapper("__set__", M_Property::__set__)},
                {"setter",
                 new InterpFunctionWrapper("setter", M_Property::setter)},
            });
        return &Property_typedef;
    }

    Typedef* get_typedef() { return _property_typedef(); }
};

/* Super */
class M_Super : public M_BaseObject {
private:
    M_BaseObject* type;
    M_BaseObject* obj_type;
    M_BaseObject* self;

public:
    M_Super(M_BaseObject* type, M_BaseObject* obj_type, M_BaseObject* self)
    {
        this->type = type;
        this->obj_type = obj_type;
        this->self = self;
    }

    virtual void mark_children(gc::GarbageCollector* gc)
    {
        gc->mark_object(type);
        gc->mark_object(obj_type);
        gc->mark_object(self);
    }

    static M_BaseObject* __new__(mtpython::vm::ThreadContext* context,
                                 const Arguments& args)
    {
        static Signature new_signature({"sub_type", "type", "obj_or_type"});
        ObjSpace* space = context->get_space();

        std::vector<M_BaseObject*> scope;
        args.parse("__new__", nullptr, new_signature, scope,
                   {space->wrap_None(), space->wrap_None()});
        M_BaseObject* w_starttype = scope[1];
        M_BaseObject* w_obj = scope[2];
        M_BaseObject* obj_type = nullptr;

        if (space->i_is(w_starttype, space->wrap_None())) {
            /* PEP 3135: calling super without arguments, using __class__
             * cell and the first parameter of the method */
            PyFrame* frame = context->top_frame();
            const std::vector<M_BaseObject*>& local_vars =
                frame->get_local_vars();
            M_BaseObject* first_arg = local_vars[0];

            PyCode* code = frame->get_pycode();
            if (!code) {
                throw InterpError(
                    space->SystemError_type(),
                    space->wrap_str(context, "super(): no code object"));
            }

            if (code->get_argcount() == 0) {
                throw InterpError(
                    space->SystemError_type(),
                    space->wrap_str(context, "super(): no arguments"));
            }

            if (!first_arg) {
                throw InterpError(
                    space->SystemError_type(),
                    space->wrap_str(context, "super(): arg[0] deleted"));
            }

            const std::vector<std::string>& co_freevars = code->get_freevars();
            int i;
            for (i = 0; i < co_freevars.size(); i++) {
                if (co_freevars[i] == "@__class__") break;
            }

            if (i == co_freevars.size()) {
                throw InterpError(
                    space->SystemError_type(),
                    space->wrap_str(context,
                                    "super(): __class__ cell not found"));
            }

            const std::vector<Cell*>& cells = frame->get_cells();
            Cell* cell = cells[code->get_ncellvars() + i];
            w_starttype = cell->get();
            obj_type = first_arg;
        }

        M_BaseObject* w_type;
        if (space->i_is(obj_type, space->wrap_None())) {
            w_type = nullptr;
            obj_type = space->wrap_None();
        } else {
            M_BaseObject* w_objtype = space->type(obj_type);
            if (space->is_true(space->issubtype(
                    w_objtype, space->get_type_by_name("type"))) &&
                space->is_true(space->issubtype(obj_type, w_starttype))) {
                w_type = obj_type;
            } else {
                w_type = w_objtype;
            }
        }

        M_Super* instance =
            new (context) M_Super(w_starttype, w_type, obj_type);
        return space->wrap(context, instance);
    }

    static M_BaseObject* __getattribute__(mtpython::vm::ThreadContext* context,
                                          M_BaseObject* obj, M_BaseObject* attr)
    {
        ObjSpace* space = context->get_space();
        std::string name = space->unwrap_str(attr);
        M_Super* as_super = static_cast<M_Super*>(obj);

        if (as_super->obj_type && name != "__class__") {
            M_BaseObject* value = space->lookup_type_starting_at(
                as_super->obj_type, as_super->type, name);

            if (value) {
                M_BaseObject* getter = space->lookup(value, "__get__");
                if (!getter) {
                    return value;
                }

                M_BaseObject* obj = nullptr;
                if (as_super->self == as_super->obj_type) {
                    obj = space->wrap_None();
                } else {
                    obj = as_super->self;
                }

                return space->get_and_call_function(
                    context, getter, {value, obj, as_super->obj_type});
            }
        }

        return nullptr;
    }

    static Typedef* _super_typedef()
    {
        static Typedef Super_typedef(
            "super", {
                         {"__new__", new InterpFunctionWrapper(
                                         "__new__", M_Super::__new__)},
                         {"__getattribute__",
                          new InterpFunctionWrapper("__getattribute__",
                                                    M_Super::__getattribute__)},
                     });
        return &Super_typedef;
    }

    Typedef* get_typedef() { return _super_typedef(); }
};

class M_Zip : public M_BaseObject {
public:
    M_Zip(ObjSpace* space) {}

    static M_BaseObject* __new__(mtpython::vm::ThreadContext* context,
                                 const Arguments& args)
    {
        static Signature new_signature({"type"});
        ObjSpace* space = context->get_space();

        std::vector<M_BaseObject*> scope;
        args.parse("__new__", nullptr, new_signature, scope);

        M_Zip* instance = new (context) M_Zip(space);
        return space->wrap(context, instance);
    }

    static M_BaseObject* __iter__(mtpython::vm::ThreadContext* context,
                                  M_BaseObject* self)
    {
        ObjSpace* space = context->get_space();
        return space->wrap(context, self);
    }

    static M_BaseObject* __next__(mtpython::vm::ThreadContext* context,
                                  M_BaseObject* self)
    {
        ObjSpace* space = context->get_space();
        return space->wrap_None();
    }

    static Typedef* _zip_typedef()
    {
        static Typedef Zip_typedef(
            "zip", {
                       {"__new__",
                        new InterpFunctionWrapper("__new__", M_Zip::__new__)},
                       {"__iter__",
                        new InterpFunctionWrapper("__iter__", M_Zip::__iter__)},
                       {"__next__",
                        new InterpFunctionWrapper("__next__", M_Zip::__next__)},
                   });

        return &Zip_typedef;
    }

    Typedef* get_typedef() { return _zip_typedef(); }
};

BuiltinsModule::BuiltinsModule(ObjSpace* space, M_BaseObject* name)
    : BuiltinModule(space, name)
{
    /* constants */
    add_def("None", space->wrap_None());
    add_def("True", space->wrap_True());
    add_def("False", space->wrap_False());
    add_def("NotImplemented", space->wrap_NotImplemented());

    /* builtin type */
    std::vector<std::string> builtin_type_names = {
        "bool", "bytearray", "bytes",     "dict", "frozenset",
        "int",  "object",    "set",       "str",  "tuple",
        "list", "type",      "memoryview"};
    for (const auto& name : builtin_type_names) {
        add_def(name, space->get_type_by_name(name));
    }

    /* builtin exceptions */
#define ADD_EXCEPTION(name) \
    add_def(#name, BaseException::get_bltin_exception_type(space, #name))

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
    ADD_EXCEPTION(SyntaxError);

    add_def("__doc__",
            new InterpDocstringWrapper("Built-in functions, exceptions, "
                                       "and other objects.\n\nNoteworthy: "
                                       "None is the `nil' object; Ellipsis "
                                       "represents `...' in slices."));
    add_def("__import__",
            new InterpFunctionWrapper("__import__", builtin___import__,
                                      Signature({"name", "globals", "locals",
                                                 "from_list", "level"})));
    add_def("__build_class__",
            new InterpFunctionWrapper(
                "__build_class__", builtin___build_class__,
                Signature({"func", "name"}, "bases", "kwargs", {})));

    add_def("abs", new InterpFunctionWrapper("abs", builtin_abs));
    add_def("classmethod",
            space->get_typeobject(ClassMethod::_classmethod_typedef()));
    add_def("compile", new InterpFunctionWrapper(
                           "compile", builtin_compile,
                           Signature({"source", "filename", "mode", "flags",
                                      "dont_inherit", "optimize"})));
    add_def("getattr", new InterpFunctionWrapper("getattr", builtin_getattr));
    add_def("globals", new InterpFunctionWrapper("globals", builtin_globals));
    add_def("isinstance",
            new InterpFunctionWrapper("isinstance", builtin_isinstance));
    add_def("issubclass",
            new InterpFunctionWrapper("issubclass", builtin_issubclass));
    add_def("iter", new InterpFunctionWrapper("iter", builtin_iter));
    add_def("len", new InterpFunctionWrapper("len", builtin_len));
    add_def("open", space->getattr_str(space->get__io(), "open"));
    add_def("print", new InterpFunctionWrapper("print", builtin_print,
                                               Signature("args", "kwargs")));
    add_def("property", space->get_typeobject(M_Property::_property_typedef()));
    add_def("range", space->get_typeobject(M_Range::_range_typedef()));
    add_def("reversed",
            new InterpFunctionWrapper("reversed", builtin_reversed));
    add_def("staticmethod",
            space->get_typeobject(StaticMethod::_staticmethod_typedef()));
    add_def("super", space->get_typeobject(M_Super::_super_typedef()));
    add_def("zip", space->get_typeobject(M_Zip::_zip_typedef()));
}
} // namespace modules

} // namespace mtpython
