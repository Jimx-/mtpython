#include "modules/_weakref/weakrefmodule.h"
#include "interpreter/gateway.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"
#include "interpreter/error.h"
#include "interpreter/pyframe.h"

using namespace mtpython::modules;
using namespace mtpython::objects;
using namespace mtpython::interpreter;
using namespace mtpython::vm;

class M_Ref : public M_BaseObject {
private:
    M_BaseObject* obj;
    M_BaseObject* callback;
public:
    M_Ref(mtpython::objects::ObjSpace* space, M_BaseObject* obj, M_BaseObject* callback) : obj(obj), callback(callback)
    {
    }

    static M_BaseObject* __new__(ThreadContext* context, const Arguments& args)
    {
        static Signature new_signature({ "type", "obj", "callback" });

        ObjSpace* space = context->get_space();
        M_BaseObject* None = space->wrap_None();
        std::vector<M_BaseObject*> scope;
        args.parse("__new__", nullptr, new_signature, scope, { None, None });

        M_BaseObject* obj = scope[1];
        M_BaseObject* callback = scope[2];
        M_BaseObject* ref = new M_Ref(space, obj, callback);

        return space->wrap(context, ref);
    }

    static M_BaseObject* __init__(ThreadContext* context, const Arguments& args)
    {
        return nullptr;
    }

    static M_BaseObject* __call__(ThreadContext* context, M_BaseObject* self)
    {
        M_Ref* ref = static_cast<M_Ref*>(self);
        return ref->obj;
    }

    Typedef* get_typedef();
};

static Typedef ref_typedef("weakref", {
    {"__new__",  new InterpFunctionWrapper("__new__", M_Ref::__new__)},
    {"__init__", new InterpFunctionWrapper("__init__", M_Ref::__init__)},
    {"__call__", new InterpFunctionWrapper("__call__", M_Ref::__call__)},
});

Typedef* M_Ref::get_typedef()
{
    return &ref_typedef;
}

WeakrefModule::WeakrefModule(mtpython::objects::ObjSpace* space, M_BaseObject* name) : BuiltinModule(space, name)
{
    add_def("ref", space->get_typeobject(&ref_typedef));
}

