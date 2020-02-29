#include <string>
#include <iostream>
#include <unordered_map>
#include <functional>
#include <assert.h>

#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "objects/std/unicode_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

M_StdUnicodeObject::M_StdUnicodeObject(const std::string& s) { value = s; }

Typedef* M_StdUnicodeObject::_str_typedef()
{
    static mtpython::interpreter::Typedef str_typedef(
        "str",
        {
            {"__repr__", new InterpFunctionWrapper(
                             "__repr__", M_StdUnicodeObject::__repr__)},
            {"__str__",
             new InterpFunctionWrapper("__str__", M_StdUnicodeObject::__str__)},
            {"__hash__", new InterpFunctionWrapper(
                             "__hash__", M_StdUnicodeObject::__hash__)},
            {"__eq__",
             new InterpFunctionWrapper("__eq__", M_StdUnicodeObject::__eq__)},
            {"__iter__", new InterpFunctionWrapper(
                             "__iter__", M_StdUnicodeObject::__iter__)},
        });

    return &str_typedef;
}

Typedef* M_StdUnicodeObject::get_typedef() { return _str_typedef(); }

bool M_StdUnicodeObject::i_is(ObjSpace* space, M_BaseObject* other)
{
    if (!M_STDUNICODEOBJECT(other)) return false;

    return this == other;
}

M_BaseObject* M_StdUnicodeObject::__iter__(mtpython::vm::ThreadContext* context,
                                           M_BaseObject* self)
{
    return context->get_space()->new_seqiter(context, self);
}

M_BaseObject* M_StdUnicodeObject::__repr__(mtpython::vm::ThreadContext* context,
                                           M_BaseObject* self)
{
    M_StdUnicodeObject* as_str = M_STDUNICODEOBJECT(self);
    assert(as_str);

    return context->get_space()->wrap_str(context, "'" + as_str->value + "'");
}

M_BaseObject* M_StdUnicodeObject::__str__(mtpython::vm::ThreadContext* context,
                                          M_BaseObject* self)
{
    return self;
}

M_BaseObject* M_StdUnicodeObject::__hash__(mtpython::vm::ThreadContext* context,
                                           M_BaseObject* self)
{
    M_StdUnicodeObject* as_str = M_STDUNICODEOBJECT(self);
    std::hash<std::string> hash_fn;
    std::size_t hash = hash_fn(as_str->value);

    return context->get_space()->wrap_int(context, hash);
}

M_BaseObject* M_StdUnicodeObject::__eq__(mtpython::vm::ThreadContext* context,
                                         mtpython::objects::M_BaseObject* self,
                                         mtpython::objects::M_BaseObject* other)
{
    M_StdUnicodeObject* self_as_str = M_STDUNICODEOBJECT(self);
    M_StdUnicodeObject* other_as_str = M_STDUNICODEOBJECT(other);

    if (!self_as_str || !other_as_str)
        return context->get_space()->new_bool(false);

    return context->get_space()->new_bool(self_as_str->value ==
                                          other_as_str->value);
}

void M_StdUnicodeObject::dbg_print() { std::cout << value; }
