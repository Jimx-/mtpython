#include "interpreter/descriptor.h"
#include "interpreter/gateway.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;

Typedef GetSetDescriptor_typedef("getset_descriptor", {
    { "__get__", new InterpFunctionWrapper("__get__", GetSetDescriptor::__get__) },
});

Typedef* GetSetDescriptor::get_typedef()
{
    return &GetSetDescriptor_typedef;
}

M_BaseObject* GetSetDescriptor::__get__(mtpython::vm::ThreadContext* context, M_BaseObject* self,  M_BaseObject* obj, M_BaseObject* cls)
{
    ObjSpace* space = context->get_space();
    GetSetDescriptor* descr = static_cast<GetSetDescriptor*>(self);
    if (space->i_is(obj, space->wrap_None()) && !space->i_is(cls, space->wrap_None())) {
        return self;
    }

    return descr->getter(context, obj);
}
