#include "objects/std/iter_object.h"
#include "interpreter/gateway.h"
#include "interpreter/error.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;
using namespace mtpython::vm;

static mtpython::interpreter::Typedef tuple_iter_typedef("tuple_iterator", {
    { "__next__", new InterpFunctionWrapper("__next__", M_StdTupleIterObject::__next__) },
});

mtpython::interpreter::Typedef* M_StdTupleIterObject::get_typedef()
{
    return &tuple_iter_typedef;
}

M_BaseObject* M_StdTupleIterObject::__next__(ThreadContext* context, M_BaseObject* self)
{
    ObjSpace*space = context->get_space();
    M_StdTupleIterObject* as_iter = dynamic_cast<M_StdTupleIterObject*>(self);
    if (!as_iter) throw InterpError(space->TypeError_type(), space->wrap_str("object is not tuple iterator"));

    if (as_iter->index == as_iter->items.size()) {
        throw InterpError(space->StopIteration_type(), space->wrap_None());
    }

    M_BaseObject* item = as_iter->items[as_iter->index];
    as_iter->index++;

    return item;
}

