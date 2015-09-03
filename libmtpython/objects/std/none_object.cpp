#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "objects/std/none_object.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;

Typedef NoneType_typedef("NoneType", {
    { "__repr__", new InterpFunctionWrapper("__repr__", M_StdNoneObject::__repr__) },
});

Typedef* M_StdNoneObject::get_typedef()
{
    return &NoneType_typedef;
}

M_BaseObject* M_StdNoneObject::__repr__(mtpython::vm::ThreadContext* context, M_BaseObject* self)
{
    return context->get_space()->wrap_str("None");
}
