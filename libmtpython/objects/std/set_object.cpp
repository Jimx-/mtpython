#include <string>
#include <unordered_set>

#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "interpreter/error.h"
#include "objects/std/set_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;
using namespace mtpython::vm;

M_BaseObject* M_StdSetObject::_discard(ThreadContext* context,
                                       M_BaseObject* item)
{
    M_BaseObject* result = nullptr;
    lock();
    auto iter = set.find(item);
    if (iter != set.end()) {
        set.erase(iter);
        result = item;
    }
    unlock();

    return result;
}

bool M_StdSetObject::i_issubset(M_StdSetObject* other)
{
    for (auto& item : set) {
        auto got = other->set.find(item);
        if (got == other->set.end()) return false;
    }

    return true;
}

M_BaseObject* M_StdSetObject::__new__(mtpython::vm::ThreadContext* context,
                                      const Arguments& args)
{
    ObjSpace* space = context->get_space();
    M_BaseObject* instance = new (context) M_StdSetObject(space);
    return space->wrap(context, instance);
}

M_BaseObject* M_StdSetObject::__init__(mtpython::vm::ThreadContext* context,
                                       const Arguments& args)
{
    ObjSpace* space = context->get_space();
    return space->wrap_None();
}

M_BaseObject* M_StdSetObject::__repr__(mtpython::vm::ThreadContext* context,
                                       M_BaseObject* self)
{
    ObjSpace* space = context->get_space();
    M_StdSetObject* as_set = static_cast<M_StdSetObject*>(self);

    std::string str = "{";
    int i = 0;
    as_set->lock();
    for (auto& item : as_set->set) {
        if (i > 0) str += ", ";
        M_BaseObject* repr_item = space->repr(item);
        str += space->unwrap_str(repr_item);
        i++;
    }
    as_set->unlock();
    str += "}";

    return space->wrap_str(context, str);
}

M_BaseObject* M_StdSetObject::__contains__(mtpython::vm::ThreadContext* context,
                                           M_BaseObject* self,
                                           M_BaseObject* obj)
{
    ObjSpace* space = context->get_space();
    M_StdSetObject* as_set = static_cast<M_StdSetObject*>(self);

    as_set->lock();
    auto got = as_set->set.find(obj);
    bool result = got != as_set->set.end();
    as_set->unlock();

    return space->new_bool(result);
}

M_BaseObject* M_StdSetObject::__le__(mtpython::vm::ThreadContext* context,
                                     M_BaseObject* self, M_BaseObject* other)
{
    ObjSpace* space = context->get_space();

    M_StdSetObject* other_as_set = dynamic_cast<M_StdSetObject*>(other);
    if (!other_as_set) return nullptr;
    M_StdSetObject* self_as_set = static_cast<M_StdSetObject*>(self);

    if (self_as_set->set.size() > other_as_set->set.size())
        return space->wrap_False();
    return space->new_bool(self_as_set->i_issubset(other_as_set));
}

M_BaseObject* M_StdSetObject::__iter__(mtpython::vm::ThreadContext* context,
                                       M_BaseObject* self)
{
    M_StdSetObject* as_set = static_cast<M_StdSetObject*>(self);
    return new (context) M_StdSetIterObject(as_set);
}

Typedef* M_StdSetObject::_set_typedef()
{
    static Typedef set_typedef(
        "set",
        {
            {"__new__",
             new InterpFunctionWrapper("__new__", M_StdSetObject::__new__)},
            {"__init__",
             new InterpFunctionWrapper("__init__", M_StdSetObject::__init__)},
            {"__repr__",
             new InterpFunctionWrapper("__repr__", M_StdSetObject::__repr__)},
            {"__contains__", new InterpFunctionWrapper(
                                 "__contains__", M_StdSetObject::__contains__)},
            {"__le__",
             new InterpFunctionWrapper("__le__", M_StdSetObject::__le__)},
            {"__iter__",
             new InterpFunctionWrapper("__iter__", M_StdSetObject::__iter__)},
            {"add", new InterpFunctionWrapper("add", M_StdSetObject::add)},
            {"remove",
             new InterpFunctionWrapper("remove", M_StdSetObject::remove)},
            {"discard",
             new InterpFunctionWrapper("discard", M_StdSetObject::discard)},
        });

    return &set_typedef;
}

Typedef* M_StdSetObject::get_typedef() { return _set_typedef(); }

M_BaseObject* M_StdSetObject::add(mtpython::vm::ThreadContext* context,
                                  M_BaseObject* self, M_BaseObject* item)
{
    M_StdSetObject* as_set = static_cast<M_StdSetObject*>(self);

    as_set->lock();
    as_set->set.insert(item);
    as_set->unlock();

    return context->get_space()->wrap_None();
}

M_BaseObject* M_StdSetObject::remove(mtpython::vm::ThreadContext* context,
                                     M_BaseObject* self, M_BaseObject* item)
{
    M_StdSetObject* as_set = static_cast<M_StdSetObject*>(self);

    M_BaseObject* result = as_set->_discard(context, item);
    if (!result) {
        throw InterpError(context->get_space()->KeyError_type(), item);
    }

    return nullptr;
}

static Typedef set_iterator_typedef(
    "set_iterator",
    {
        {"__next__",
         new InterpFunctionWrapper("__next__", M_StdSetIterObject::__next__)},
    });

M_StdSetIterObject::M_StdSetIterObject(M_StdSetObject* set)
{
    this->set = set;
    set->lock();
    size = set->size();
    iter = set->begin();
    index = size == 0 ? -1 : 0;
    set->unlock();
}

M_BaseObject* M_StdSetIterObject::__next__(mtpython::vm::ThreadContext* context,
                                           M_BaseObject* self)
{
    ObjSpace* space = context->get_space();
    M_StdSetIterObject* iter = static_cast<M_StdSetIterObject*>(self);

    if (iter->index < 0)
        throw InterpError(space->StopIteration_type(), space->wrap_None());
    iter->set->lock();

    if (iter->size != iter->set->size()) {
        iter->set->unlock();
        throw InterpError(
            space->IndexError_type(),
            space->wrap_str(context, "Set changed size during iteration"));
    }

    M_BaseObject* item = *(iter->iter);

    iter->iter++;
    iter->index++;

    if (iter->index == iter->size) {
        iter->index = -1;
    }
    iter->set->unlock();

    return item;
}

Typedef* M_StdSetIterObject::get_typedef() { return &set_iterator_typedef; }

M_BaseObject* M_StdSetObject::discard(mtpython::vm::ThreadContext* context,
                                      M_BaseObject* self, M_BaseObject* item)
{
    (static_cast<M_StdSetObject*>(self))->_discard(context, item);

    return nullptr;
}
