#include <string>
#include <unordered_map>
#include <assert.h>

#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "interpreter/error.h"
#include "objects/std/type_object.h"
#include "objects/std/dict_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

M_BaseObject* M_StdDictObject::getitem(M_BaseObject* key)
{
    auto got = dict.find(key);
    if (got == dict.end()) return nullptr;

    return got->second;
}

void M_StdDictObject::setitem(M_BaseObject* key, M_BaseObject* value)
{
    dict[key] = value;
}

M_BaseObject* M_StdDictObject::__repr__(mtpython::vm::ThreadContext* context,
                                        M_BaseObject* self)
{
    ObjSpace* space = context->get_space();
    M_StdDictObject* as_dict = dynamic_cast<M_StdDictObject*>(self);
    assert(as_dict);
    std::string str;

    as_dict->lock();
    str += "{";
    int i = 0;
    for (auto& item : as_dict->dict) {
        if (i > 0) str += ", ";
        M_BaseObject* repr_item = space->repr(item.first);
        str += space->unwrap_str(repr_item);
        str += ": ";
        repr_item = space->repr(item.second);
        str += space->unwrap_str(repr_item);
        i++;
    }
    str += "}";
    as_dict->unlock();

    return space->wrap_str(context, str);
}

M_BaseObject* M_StdDictObject::__getitem__(mtpython::vm::ThreadContext* context,
                                           M_BaseObject* obj, M_BaseObject* key)
{
    ObjSpace* space = context->get_space();
    M_StdDictObject* as_dict = dynamic_cast<M_StdDictObject*>(obj);
    assert(as_dict);

    as_dict->lock();
    M_BaseObject* value = as_dict->getitem(key);
    as_dict->unlock();

    if (!value) throw InterpError(space->KeyError_type(), key);
    return value;
}

M_BaseObject* M_StdDictObject::__setitem__(mtpython::vm::ThreadContext* context,
                                           M_BaseObject* obj, M_BaseObject* key,
                                           M_BaseObject* value)
{
    M_StdDictObject* as_dict = static_cast<M_StdDictObject*>(obj);

    as_dict->lock();
    as_dict->setitem(key, value);
    as_dict->unlock();

    return nullptr;
}

M_BaseObject* M_StdDictObject::__len__(mtpython::vm::ThreadContext* context,
                                       M_BaseObject* self)
{
    M_StdDictObject* as_dict = static_cast<M_StdDictObject*>(self);

    as_dict->lock();
    std::size_t size = as_dict->dict.size();
    as_dict->unlock();

    return context->get_space()->wrap_int(context, (int)size);
}

/* copy the (key, value) pairs of a mapping into dict */
static void copy_mapping(mtpython::vm::ThreadContext* context,
                         ObjSpace* space, M_BaseObject* source,
                         M_StdDictObject* dict)
{
    M_BaseObject* keys_meth = space->getattr_str(source, "keys");
    M_BaseObject* wrapped_keys = space->call_function(context, keys_meth, {});
    std::vector<M_BaseObject*> keys;
    space->unwrap_tuple(wrapped_keys, keys);
    for (M_BaseObject* key : keys) {
        dict->setitem(key, space->getitem(source, key));
    }
}

/* collect an iterable into a vector via the iter/next protocol: works
   for tuples and lists alike, and raises TypeError for
   non-iterables */
static void collect_iterable(mtpython::vm::ThreadContext* context,
                             ObjSpace* space, M_BaseObject* iterable,
                             std::vector<M_BaseObject*>& list)
{
    M_BaseObject* iter = space->iter(iterable);
    while (true) {
        try {
            list.push_back(space->next(iter));
        } catch (InterpError& e) {
            if (!e.match(space, space->StopIteration_type())) throw e;
            break;
        }
    }
}

M_BaseObject* M_StdDictObject::__new__(mtpython::vm::ThreadContext* context,
                                       const Arguments& args)
{
    static Signature new_signature({"type"}, "args", "kwargs", {});

    ObjSpace* space = context->get_space();
    std::vector<M_BaseObject*> scope;
    args.parse("dict", nullptr, new_signature, scope);

    M_BaseObject* wrapped_type = scope[0];
    M_StdTypeObject* type_obj = dynamic_cast<M_StdTypeObject*>(wrapped_type);
    if (!type_obj)
        throw InterpError::format(space, space->TypeError_type(),
                                  "dict.__new__(X): X is not a type object");

    std::vector<M_BaseObject*> extra;
    space->unwrap_tuple(scope[1], extra);
    if (extra.size() > 1)
        throw InterpError::format(space, space->TypeError_type(),
                                  "dict() takes at most 1 argument (%d given)",
                                  (int)extra.size());

    M_StdDictObject* dict = new (context) M_StdDictObject(space);

    if (extra.size() == 1) {
        M_BaseObject* source = extra[0];
        bool is_mapping = true;
        try {
            space->getattr_str(source, "keys");
        } catch (InterpError& e) {
            if (!e.match(space, space->AttributeError_type())) throw e;
            is_mapping = false;
        }

        if (is_mapping) {
            copy_mapping(context, space, source, dict);
        } else { /* iterable of (key, value) pairs */
            std::vector<M_BaseObject*> items;
            collect_iterable(context, space, source, items);
            for (std::size_t i = 0; i < items.size(); i++) {
                std::vector<M_BaseObject*> pair;
                collect_iterable(context, space, items[i], pair);
                if (pair.size() != 2)
                    throw InterpError::format(
                        space, space->ValueError_type(),
                        "dictionary update sequence element #%d has length "
                        "%d; 2 is required",
                        (int)i, (int)pair.size());
                dict->setitem(pair[0], pair[1]);
            }
        }
    }

    /* keyword arguments */
    copy_mapping(context, space, scope[2], dict);

    return space->wrap(context, dict);
}

mtpython::interpreter::Typedef* M_StdDictObject::_dict_typedef()
{
    static mtpython::interpreter::Typedef dict_typedef(
        "dict",
        {
            {"__new__",
             new InterpFunctionWrapper("__new__", M_StdDictObject::__new__)},
            {"__len__",
             new InterpFunctionWrapper("__len__", M_StdDictObject::__len__)},
            {"__repr__",
             new InterpFunctionWrapper("__repr__", M_StdDictObject::__repr__)},
            {"__getitem__", new InterpFunctionWrapper(
                                "__getitem__", M_StdDictObject::__getitem__)},
            {"__setitem__", new InterpFunctionWrapper(
                                "__setitem__", M_StdDictObject::__setitem__)},
            {"__contains__",
             new InterpFunctionWrapper("__contains__",
                                       M_StdDictObject::__contains__)},
            {"keys", new InterpFunctionWrapper("keys", M_StdDictObject::keys)},
            {"values",
             new InterpFunctionWrapper("values", M_StdDictObject::values)},
            {"items",
             new InterpFunctionWrapper("items", M_StdDictObject::items)},
        });

    return &dict_typedef;
}

mtpython::interpreter::Typedef* M_StdDictObject::get_typedef()
{
    return _dict_typedef();
}

M_BaseObject*
M_StdDictObject::__contains__(mtpython::vm::ThreadContext* context,
                              M_BaseObject* self, M_BaseObject* obj)
{
    M_StdDictObject* as_dict = static_cast<M_StdDictObject*>(self);

    as_dict->lock();
    M_BaseObject* result = context->get_space()->new_bool(
        as_dict->dict.find(obj) != as_dict->dict.end());
    as_dict->unlock();

    return result;
}

M_BaseObject* M_StdDictObject::keys(mtpython::vm::ThreadContext* context,
                                    M_BaseObject* self)
{
    M_StdDictObject* as_dict = static_cast<M_StdDictObject*>(self);
    std::vector<M_BaseObject*> keys;

    as_dict->lock();
    for (const auto& iter : as_dict->dict) {
        keys.push_back(iter.first);
    }
    as_dict->unlock();

    return context->get_space()->new_tuple(context, keys);
}

M_BaseObject* M_StdDictObject::values(mtpython::vm::ThreadContext* context,
                                      M_BaseObject* self)
{
    M_StdDictObject* as_dict = static_cast<M_StdDictObject*>(self);
    std::vector<M_BaseObject*> values;

    as_dict->lock();
    for (const auto& iter : as_dict->dict) {
        values.push_back(iter.second);
    }
    as_dict->unlock();

    return context->get_space()->new_tuple(context, values);
}

M_BaseObject* M_StdDictObject::items(mtpython::vm::ThreadContext* context,
                                     M_BaseObject* self)
{
    M_StdDictObject* as_dict = static_cast<M_StdDictObject*>(self);
    std::vector<M_BaseObject*> items;
    ObjSpace* space = context->get_space();

    as_dict->lock();
    for (const auto& iter : as_dict->dict) {
        std::vector<M_BaseObject*> item;
        item.push_back(iter.first);
        item.push_back(iter.second);
        items.push_back(space->new_tuple(context, item));
    }
    as_dict->unlock();

    return context->get_space()->new_list(context, items);
}
