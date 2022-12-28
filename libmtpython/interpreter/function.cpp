#include "interpreter/function.h"
#include "interpreter/descriptor.h"
#include "interpreter/gateway.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;
using namespace mtpython::vm;

Function::Function(ObjSpace* space, Code* code,
                   const std::vector<M_BaseObject*>& defaults,
                   M_BaseObject* globals)
    : space(space), name(code->get_name()), code(code), func_globals(globals),
      defaults(defaults)
{
    func_dict = nullptr;
    doc = nullptr;
}

Typedef* Function::get_typedef()
{
    static Typedef Function_typedef(
        "function",
        {
            {"__get__",
             new InterpFunctionWrapper("__get__", Function::__get__)},
            {"__doc__",
             new GetSetDescriptor(Function::__doc__get, Function::__doc__set)},
        });

    return &Function_typedef;
}

M_BaseObject* Function::get_dict(ObjSpace* space)
{
    if (func_dict) return func_dict;
    func_dict = space->new_dict(ThreadContext::current_thread());

    return func_dict;
}

M_BaseObject* Function::call_args(mtpython::vm::ThreadContext* context,
                                  Arguments& args)
{
    return get_code()->funcrun(context, this, args);
}

M_BaseObject* Function::call_obj_args(mtpython::vm::ThreadContext* context,
                                      M_BaseObject* obj, Arguments& args)
{
    return get_code()->funcrun_obj(context, this, obj, args);
}

M_BaseObject* Function::__get__(mtpython::vm::ThreadContext* context,
                                M_BaseObject* self, M_BaseObject* obj,
                                M_BaseObject* type)
{
    ObjSpace* space = context->get_space();
    if (!obj || space->i_is(obj, space->wrap_None())) return self;

    Function* as_func = dynamic_cast<Function*>(self);
    return space->wrap(context, new (context) Method(space, as_func, obj));
}

M_BaseObject* Function::__doc__get(mtpython::vm::ThreadContext* context,
                                   M_BaseObject* self)
{
    Function* f = static_cast<Function*>(self);
    if (!f->doc) {
        f->doc = f->code->get_docstring(context);
    }
    return f->doc;
}

void Function::__doc__set(mtpython::vm::ThreadContext* context,
                          M_BaseObject* obj, M_BaseObject* value)
{
    Function* f = static_cast<Function*>(obj);
    f->doc = value;
}

M_BaseObject* Method::call_args(mtpython::vm::ThreadContext* context,
                                Arguments& args)
{
    return space->call_obj_args(context, func, instance, args);
}

M_BaseObject* StaticMethod::__new__(mtpython::vm::ThreadContext* context,
                                    M_BaseObject* type, M_BaseObject* func)
{
    StaticMethod* static_met = new (context) StaticMethod(func);
    return context->get_space()->wrap(context, static_met);
}

M_BaseObject* StaticMethod::__get__(mtpython::vm::ThreadContext* context,
                                    const Arguments& args)
{
    static Signature get_signature({"self", "obj", "type"});
    ObjSpace* space = context->get_space();

    std::vector<M_BaseObject*> scope;
    args.parse("__get__", nullptr, get_signature, scope, {space->wrap_None()});

    StaticMethod* self = static_cast<StaticMethod*>(scope[0]);

    return space->wrap(context, self->func);
}

Typedef* StaticMethod::get_typedef() { return _staticmethod_typedef(); }

Typedef* StaticMethod::_staticmethod_typedef()
{
    static Typedef StaticMethod_typedef(
        "staticmethod",
        {
            {"__new__",
             new InterpFunctionWrapper("__new__", StaticMethod::__new__)},
            {"__get__",
             new InterpFunctionWrapper("__get__", StaticMethod::__get__)},
        });

    return &StaticMethod_typedef;
}

M_BaseObject* ClassMethod::__new__(mtpython::vm::ThreadContext* context,
                                   M_BaseObject* type, M_BaseObject* func)
{
    ClassMethod* cls_met = new (context) ClassMethod(func);
    return context->get_space()->wrap(context, cls_met);
}

M_BaseObject* ClassMethod::__get__(mtpython::vm::ThreadContext* context,
                                   const Arguments& args)
{
    static Signature get_signature({"self", "obj", "type"});
    ObjSpace* space = context->get_space();

    std::vector<M_BaseObject*> scope;
    args.parse("__get__", nullptr, get_signature, scope, {space->wrap_None()});

    ClassMethod* self = static_cast<ClassMethod*>(scope[0]);
    M_BaseObject* obj = scope[1];
    M_BaseObject* cls = scope[2];

    if (space->i_is(cls, space->wrap_None())) {
        cls = space->type(obj);
    }

    return space->wrap(context, new (context) Method(space, self->func, cls));
}

Typedef* ClassMethod::get_typedef() { return _classmethod_typedef(); }

Typedef* ClassMethod::_classmethod_typedef()
{
    static Typedef ClassMethod_typedef(
        "classmethod",
        {
            {"__new__",
             new InterpFunctionWrapper("__new__", ClassMethod::__new__)},
            {"__get__",
             new InterpFunctionWrapper("__get__", ClassMethod::__get__)},
        });

    return &ClassMethod_typedef;
}
