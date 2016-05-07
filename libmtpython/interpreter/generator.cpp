#include "interpreter/generator.h"
#include "interpreter/gateway.h"
#include "interpreter/error.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;

GeneratorIterator::GeneratorIterator(PyFrame* _frame)
{
    frame = _frame;
    space = frame->get_space();
    running = false;
}

Typedef GeneratorIterator_typedef("generator", {
    { "__iter__", new InterpFunctionWrapper("__iter__", GeneratorIterator::__iter__) },
    { "__next__", new InterpFunctionWrapper("__next__", GeneratorIterator::__next__) },
    { "send", new InterpFunctionWrapper("send", GeneratorIterator::send) },
});

Typedef* GeneratorIterator::get_typedef()
{
    return &GeneratorIterator_typedef;
}

M_BaseObject* GeneratorIterator::__iter__(vm::ThreadContext* context, M_BaseObject* self)
{
    ObjSpace* space = context->get_space();
    return space->wrap(context, self);
}

M_BaseObject* GeneratorIterator::__next__(vm::ThreadContext* context, M_BaseObject* self)
{
    ObjSpace* space = context->get_space();
    return send(context, self, space->wrap_None());
}

M_BaseObject* GeneratorIterator::send(vm::ThreadContext* context, objects::M_BaseObject* self,
                                      objects::M_BaseObject* arg)
{
    GeneratorIterator* gi = static_cast<GeneratorIterator*>(self);
    ObjSpace* space = gi->space;
    if (gi->running) {
        throw InterpError(space->ValueError_type(), space->wrap_str(context, "generator already executing"));
    }

    if (!gi->frame) {
        throw InterpError(space->StopIteration_type(), space->wrap_None());
    }

    PyFrame* frame = gi->frame;
    if (frame->get_pc() == -1) {
        if (arg && !space->i_is(arg, space->wrap_None())) {
            throw InterpError(space->TypeError_type(), space->wrap_str(context,
             "can't send non-None value to a just-started generator"));
        }
    } else if (!arg) arg = space->wrap_None();

    gi->running = true;
    M_BaseObject* result = nullptr;

    try {
       result = frame->execute_frame(arg);
    } catch (InterpError&) {
        gi->frame = nullptr;
        gi->running = false;
        throw;
    }

    gi->running = false;
    if (frame->finished_execution()) {
        gi->frame = nullptr;
        throw InterpError(space->StopIteration_type(), space->wrap_None());
    }

    return result;
}

