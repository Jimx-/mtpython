#include "objects/std/iter_object.h"
#include "interpreter/gateway.h"
#include "interpreter/error.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;
using namespace mtpython::vm;

static Typedef seq_iter_typedef("seq_iterator", {
    { "__next__", new InterpFunctionWrapper("__next__", M_StdSeqIterObject::__next__) },
});

M_BaseObject* M_StdSeqIterObject::__next__(ThreadContext* context, M_BaseObject* self)
{
    ObjSpace* space = context->get_space();

    M_StdSeqIterObject* iter = static_cast<M_StdSeqIterObject*>(self);
    iter->lock();
    if (!iter->obj) {
        throw InterpError(space->StopIteration_type(), space->wrap_None());
    }

    M_BaseObject* item;
    try {
        item = space->getitem(iter->obj, space->wrap_int(context, iter->index));
    } catch (InterpError& exc) {
        iter->obj = nullptr;
        if (!exc.match(space, space->IndexError_type())) throw exc;
        throw InterpError(space->StopIteration_type(), space->wrap_None());
    }
    iter->index++;
    iter->unlock();

    return item;
}

Typedef* M_StdSeqIterObject::get_typedef()
{
    return &seq_iter_typedef;
}

static Typedef tuple_iter_typedef("tuple_iterator", {
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
    if (!as_iter) throw InterpError(space->TypeError_type(), space->wrap_str(context, "object is not tuple iterator"));

    if (as_iter->index == as_iter->items.size()) {
        throw InterpError(space->StopIteration_type(), space->wrap_None());
    }

    M_BaseObject* item = as_iter->items[as_iter->index];
    as_iter->index++;

    return item;
}

