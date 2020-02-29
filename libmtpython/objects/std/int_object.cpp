#include <string>
#include <iostream>
#include <unordered_map>
#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "interpreter/error.h"
#include "objects/std/int_object.h"
#include "exceptions.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

M_StdIntObject::M_StdIntObject(int x) { intval = x; }

M_StdIntObject::M_StdIntObject(const std::string& x)
{
    try {
        intval = std::stoi(x, nullptr, 0);
    } catch (std::out_of_range&) {
        /* TODO: create long object */
        intval = 0;
    }
}

mtpython::interpreter::Typedef* M_StdIntObject::_int_typedef()
{
    static mtpython::interpreter::Typedef int_typedef(
        "int",
        {
            {"__repr__",
             new InterpFunctionWrapper("__repr__", M_StdIntObject::__repr__)},
            {"__str__",
             new InterpFunctionWrapper("__str__", M_StdIntObject::__repr__)},
            {"__bool__",
             new InterpFunctionWrapper("__bool__", M_StdIntObject::__bool__)},
            {"__add__",
             new InterpFunctionWrapper("__add__", M_StdIntObject::__add__)},
            {"__sub__",
             new InterpFunctionWrapper("__sub__", M_StdIntObject::__sub__)},
            {"__mul__",
             new InterpFunctionWrapper("__mul__", M_StdIntObject::__mul__)},
            {"__and__",
             new InterpFunctionWrapper("__and__", M_StdIntObject::__and__)},
            {"__eq__",
             new InterpFunctionWrapper("__eq__", M_StdIntObject::__eq__)},
            {"__ne__",
             new InterpFunctionWrapper("__ne__", M_StdIntObject::__ne__)},
            {"__lt__",
             new InterpFunctionWrapper("__lt__", M_StdIntObject::__lt__)},
            {"__le__",
             new InterpFunctionWrapper("__le__", M_StdIntObject::__le__)},
            {"__gt__",
             new InterpFunctionWrapper("__gt__", M_StdIntObject::__gt__)},
            {"__ge__",
             new InterpFunctionWrapper("__ge__", M_StdIntObject::__ge__)},
            {"__abs__",
             new InterpFunctionWrapper("__abs__", M_StdIntObject::__abs__)},
            {"__neg__",
             new InterpFunctionWrapper("__neg__", M_StdIntObject::__neg__)},
        });

    return &int_typedef;
}

mtpython::interpreter::Typedef* M_StdIntObject::get_typedef()
{
    return _int_typedef();
}

void M_StdIntObject::dbg_print() { std::cout << intval; }

M_BaseObject* M_StdIntObject::__repr__(mtpython::vm::ThreadContext* context,
                                       M_BaseObject* self)
{
    ObjSpace* space = context->get_space();
    int i = space->unwrap_int(self);
    return space->wrap_str(context, std::to_string(i));
}

M_BaseObject* M_StdIntObject::__str__(mtpython::vm::ThreadContext* context,
                                      M_BaseObject* self)
{
    ObjSpace* space = context->get_space();
    int i = space->unwrap_int(self);
    return space->wrap_str(context, std::to_string(i));
}

M_BaseObject* M_StdIntObject::__bool__(mtpython::vm::ThreadContext* context,
                                       M_BaseObject* self)
{
    ObjSpace* space = context->get_space();
    M_StdIntObject* as_int = M_STDINTOBJECT(self);
    if (!as_int)
        throw InterpError(space->TypeError_type(),
                          space->wrap_str(context, "object is not int"));

    return space->new_bool(as_int->intval != 0);
}

M_BaseObject* M_StdIntObject::__add__(mtpython::vm::ThreadContext* context,
                                      M_BaseObject* self, M_BaseObject* other)
{
    ObjSpace* space = context->get_space();

    M_StdIntObject* self_as_int = static_cast<M_StdIntObject*>(self);
    M_StdIntObject* other_as_int = dynamic_cast<M_StdIntObject*>(other);
    if (!other_as_int) return nullptr;

    int x = self_as_int->intval;
    int y = other_as_int->intval;

    int z = x + y;

    return space->wrap_int(context, z);
}

M_BaseObject* M_StdIntObject::__sub__(mtpython::vm::ThreadContext* context,
                                      M_BaseObject* self, M_BaseObject* other)
{
    ObjSpace* space = context->get_space();

    M_StdIntObject* self_as_int = static_cast<M_StdIntObject*>(self);
    M_StdIntObject* other_as_int = dynamic_cast<M_StdIntObject*>(other);
    if (!other_as_int) return nullptr;

    int x = self_as_int->intval;
    int y = other_as_int->intval;

    int z = x - y;

    return space->wrap_int(context, z);
}

M_BaseObject* M_StdIntObject::__mul__(mtpython::vm::ThreadContext* context,
                                      M_BaseObject* self, M_BaseObject* other)
{
    ObjSpace* space = context->get_space();

    M_StdIntObject* self_as_int = static_cast<M_StdIntObject*>(self);
    M_StdIntObject* other_as_int = dynamic_cast<M_StdIntObject*>(other);
    if (!other_as_int) return nullptr;

    int x = self_as_int->intval;
    int y = other_as_int->intval;

    int z = x * y;

    return space->wrap_int(context, z);
}

M_BaseObject* M_StdIntObject::__and__(mtpython::vm::ThreadContext* context,
                                      M_BaseObject* self, M_BaseObject* other)
{
    ObjSpace* space = context->get_space();

    M_StdIntObject* self_as_int = static_cast<M_StdIntObject*>(self);
    M_StdIntObject* other_as_int = dynamic_cast<M_StdIntObject*>(other);
    if (!other_as_int) return nullptr;

    int x = self_as_int->intval;
    int y = other_as_int->intval;

    int z = x & y;

    return space->wrap_int(context, z);
}

M_BaseObject* M_StdIntObject::__eq__(mtpython::vm::ThreadContext* context,
                                     mtpython::objects::M_BaseObject* self,
                                     mtpython::objects::M_BaseObject* other)
{
    ObjSpace* space = context->get_space();

    M_StdIntObject* self_as_int = static_cast<M_StdIntObject*>(self);
    M_StdIntObject* other_as_int = dynamic_cast<M_StdIntObject*>(other);
    if (!other_as_int) return nullptr;

    int x = self_as_int->intval;
    int y = other_as_int->intval;

    return space->new_bool(x == y);
}

M_BaseObject* M_StdIntObject::__ne__(mtpython::vm::ThreadContext* context,
                                     mtpython::objects::M_BaseObject* self,
                                     mtpython::objects::M_BaseObject* other)
{
    ObjSpace* space = context->get_space();

    M_StdIntObject* self_as_int = static_cast<M_StdIntObject*>(self);
    M_StdIntObject* other_as_int = dynamic_cast<M_StdIntObject*>(other);
    if (!other_as_int) return nullptr;

    int x = self_as_int->intval;
    int y = other_as_int->intval;

    return space->new_bool(x != y);
}

M_BaseObject* M_StdIntObject::__lt__(mtpython::vm::ThreadContext* context,
                                     mtpython::objects::M_BaseObject* self,
                                     mtpython::objects::M_BaseObject* other)
{
    ObjSpace* space = context->get_space();

    M_StdIntObject* self_as_int = static_cast<M_StdIntObject*>(self);
    M_StdIntObject* other_as_int = dynamic_cast<M_StdIntObject*>(other);
    if (!other_as_int) return nullptr;

    int x = self_as_int->intval;
    int y = other_as_int->intval;

    return space->new_bool(x < y);
}

M_BaseObject* M_StdIntObject::__le__(mtpython::vm::ThreadContext* context,
                                     mtpython::objects::M_BaseObject* self,
                                     mtpython::objects::M_BaseObject* other)
{
    ObjSpace* space = context->get_space();

    M_StdIntObject* self_as_int = static_cast<M_StdIntObject*>(self);
    M_StdIntObject* other_as_int = dynamic_cast<M_StdIntObject*>(other);
    if (!other_as_int) return nullptr;

    int x = self_as_int->intval;
    int y = other_as_int->intval;

    return space->new_bool(x <= y);
}

M_BaseObject* M_StdIntObject::__gt__(mtpython::vm::ThreadContext* context,
                                     mtpython::objects::M_BaseObject* self,
                                     mtpython::objects::M_BaseObject* other)
{
    ObjSpace* space = context->get_space();

    M_StdIntObject* self_as_int = static_cast<M_StdIntObject*>(self);
    M_StdIntObject* other_as_int = dynamic_cast<M_StdIntObject*>(other);
    if (!other_as_int) return nullptr;

    int x = self_as_int->intval;
    int y = other_as_int->intval;

    return space->new_bool(x > y);
}

M_BaseObject* M_StdIntObject::__ge__(mtpython::vm::ThreadContext* context,
                                     mtpython::objects::M_BaseObject* self,
                                     mtpython::objects::M_BaseObject* other)
{
    ObjSpace* space = context->get_space();

    M_StdIntObject* self_as_int = static_cast<M_StdIntObject*>(self);
    M_StdIntObject* other_as_int = dynamic_cast<M_StdIntObject*>(other);
    if (!other_as_int) return nullptr;

    int x = self_as_int->intval;
    int y = other_as_int->intval;

    return space->new_bool(x >= y);
}

M_BaseObject* M_StdIntObject::__abs__(mtpython::vm::ThreadContext* context,
                                      M_BaseObject* self)
{
    ObjSpace* space = context->get_space();
    M_StdIntObject* as_int = M_STDINTOBJECT(self);

    int x = as_int->intval;
    if (x < 0) x = -x;

    return space->wrap_int(context, x);
}

M_BaseObject* M_StdIntObject::__neg__(mtpython::vm::ThreadContext* context,
                                      M_BaseObject* self)
{
    ObjSpace* space = context->get_space();
    M_StdIntObject* as_int = M_STDINTOBJECT(self);

    return space->wrap_int(context, -(as_int->intval));
}
