#include <list>

#include "objects/std/type_object.h"
#include "objects/std/object_object.h"
#include "interpreter/gateway.h"
#include "interpreter/descriptor.h"
#include "interpreter/error.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

M_StdTypeObject::M_StdTypeObject(
    ObjSpace* space, M_BaseObject* cls, std::string& name,
    const std::vector<M_BaseObject*>& bases,
    const std::unordered_map<std::string, M_BaseObject*>& dict)
    : space(space), cls(cls), name(name), bases(bases), dict(dict)
{
    _has_dict = false;
    wrapped_dict = nullptr;
    init_mro();
}

mtpython::interpreter::Typedef* M_StdTypeObject::get_typedef()
{
    return _type_typedef();
}

mtpython::interpreter::Typedef* M_StdTypeObject::_type_typedef()
{
    static mtpython::interpreter::Typedef type_typedef(
        "type",
        {
            {"__new__",
             new InterpFunctionWrapper("__new__", M_StdTypeObject::__new__)},
            {"__call__",
             new InterpFunctionWrapper("__call__", M_StdTypeObject::__call__)},
            {"__getattribute__",
             new InterpFunctionWrapper("__getattribute__",
                                       M_StdTypeObject::__getattribute__)},
            {"__repr__",
             new InterpFunctionWrapper("__repr__", M_StdTypeObject::__repr__)},
            {"__subclasscheck__",
             new InterpFunctionWrapper("__subclasscheck__",
                                       M_StdTypeObject::__subclasscheck__)},
            {"__mro__", new GetSetDescriptor(M_StdTypeObject::__mro__get)},
            {"__subclasses__",
             new InterpFunctionWrapper("__subclasses__",
                                       M_StdTypeObject::__subclasses__)},
        });

    return &type_typedef;
}

void M_StdTypeObject::init_mro()
{
    std::list<std::vector<M_BaseObject*>> orderlist;
    std::vector<M_BaseObject*> _bases;
    _bases.push_back(this);
    for (auto base : bases) {
        _bases.push_back(base);
        M_StdTypeObject* typeobj = dynamic_cast<M_StdTypeObject*>(base);
        if (!typeobj) continue;

        orderlist.push_back(typeobj->mro);
    }

    orderlist.push_back(_bases);

    M_BaseObject* candidate;
    while (true) {
        candidate = nullptr;
        for (auto& candidates : orderlist) {
            if (candidates.size() > 0) {
                candidate = candidates[0];

                bool ok = true;
                for (auto& lst : orderlist) {
                    if (lst.size() <= 1) continue;
                    for (auto it = lst.begin() + 1; it != lst.end(); it++) {
                        if (*it == candidate) ok = false;
                    }
                }

                if (ok) break;
            }
        }

        if (!candidate) return;

        mro.push_back(candidate);
        for (auto it = orderlist.rbegin(); it != orderlist.rend(); it++) {
            if ((*it).size() == 0) continue;
            if ((*it)[0] == candidate) (*it).erase((*it).begin());
        }
    }
}

M_BaseObject* M_StdTypeObject::get_dict(ObjSpace* space)
{
    if (wrapped_dict) return wrapped_dict;

    mtpython::vm::ThreadContext* context = vm::ThreadContext::current_thread();
    wrapped_dict = space->new_dict(context);

    for (auto& it : dict) {
        space->setitem(wrapped_dict, space->wrap_str(context, it.first),
                       space->wrap(context, it.second));
    }

    return wrapped_dict;
}

M_BaseObject* M_StdTypeObject::get_dict_value(ObjSpace* space,
                                              const std::string& attr)
{
    auto got = dict.find(attr);
    if (got == dict.end()) return nullptr;

    return got->second;
}

bool M_StdTypeObject::set_dict_value(ObjSpace* space, const std::string& attr,
                                     M_BaseObject* value)
{
    dict[attr] = value;
    return true;
}

M_BaseObject* M_StdTypeObject::lookup(const std::string& name)
{
    for (auto base : mro) {
        M_BaseObject* value = base->get_dict_value(space, name);
        if (value) {
            return value;
        }
    }

    return nullptr;
}

M_BaseObject* M_StdTypeObject::lookup_starting_at(M_BaseObject* start,
                                                  const std::string& name)
{
    int found = false;
    for (auto base : mro) {
        if (base == start) {
            found = true;
        } else if (found) {
            M_BaseObject* value = base->get_dict_value(space, name);
            if (value) {
                return value;
            }
        }
    }

    return nullptr;
}

M_BaseObject* M_StdTypeObject::lookup_cls(const std::string& attr,
                                          M_BaseObject*& cls)
{
    for (auto base : mro) {
        M_BaseObject* value = base->get_dict_value(space, attr);
        if (value) {
            cls = base;
            return value;
        }
    }

    return nullptr;
}

bool M_StdTypeObject::issubtype(M_BaseObject* type)
{
    for (const auto base : mro) {
        if (base == type) return true;
    }

    return false;
}

M_BaseObject* StdTypedefCache::build(mtpython::interpreter::Typedef* key)
{
    std::unordered_map<std::string, M_BaseObject*>& dict = key->get_dict();

    std::vector<M_BaseObject*> wrapped_bases;

    if (key == M_StdObjectObject::_object_typedef())
        wrapped_bases.clear();
    else {
        std::vector<mtpython::interpreter::Typedef*>& typedef_bases =
            key->get_bases();
        if (typedef_bases.size() != 0) {
            for (mtpython::interpreter::Typedef* def : typedef_bases) {
                wrapped_bases.push_back(space->get_typeobject(def));
            }
        } else {
            wrapped_bases.push_back(
                space->get_typeobject(M_StdObjectObject::_object_typedef()));
        }
    }

    std::unordered_map<std::string, M_BaseObject*> wrapped_dict;
    for (auto it = dict.begin(); it != dict.end(); it++) {
        wrapped_dict[it->first] =
            space->wrap(vm::ThreadContext::current_thread(), it->second);
    }

    M_StdTypeObject* wrapped_type = new (vm::ThreadContext::current_thread())
        M_StdTypeObject(space, space->get_type_by_name("type"), key->get_name(),
                        wrapped_bases, wrapped_dict);

    return wrapped_type;
}

void M_StdTypeObject::ready() {}

void M_StdTypeObject::mark_children(gc::GarbageCollector* gc)
{
    for (const auto& obj : bases)
        gc->mark_object(obj);
    for (const auto& [k, v] : dict)
        gc->mark_object_maybe(v);

    if (wrapped_dict) gc->mark_object(wrapped_dict);

    for (const auto& obj : mro)
        gc->mark_object(obj);
    for (const auto& obj : subclasses)
        gc->mark_object(obj);
    gc->mark_object(cls);
}

M_BaseObject* M_StdTypeObject::__new__(mtpython::vm::ThreadContext* context,
                                       const Arguments& args)
{
    static Signature new_signature({"type", "name", "bases", "dict"});

    ObjSpace* space = context->get_space();

    std::vector<M_BaseObject*> scope;
    args.parse("__new__", nullptr, new_signature, scope, {nullptr, nullptr});

    M_BaseObject* wrapped_type = scope[0];
    M_BaseObject* wrapped_name = scope[1];
    M_BaseObject* wrapped_bases = scope[2];
    M_BaseObject* wrapped_dict = scope[3];

    /* special case: type(x) */
    if (space->i_is(wrapped_type, space->get_type_by_name("type")) &&
        !wrapped_bases && !wrapped_dict) {
        return space->type(wrapped_name);
    }

    if (wrapped_bases && !wrapped_dict) {
        throw InterpError(
            space->TypeError_type(),
            space->wrap_str(context, "type() takes 1 or 3 arguments"));
    }

    std::vector<M_BaseObject*> bases;
    if (wrapped_bases) {
        space->unwrap_tuple(wrapped_bases, bases);
    }
    if (bases.size() == 0) bases.push_back(space->get_type_by_name("object"));

    std::string name = space->unwrap_str(wrapped_name);
    M_BaseObject* keys_impl = space->getattr_str(wrapped_dict, "keys");
    if (!keys_impl) {
        throw InterpError::format(space, space->TypeError_type(),
                                  "type() argument 3 must be dict, not %s",
                                  space->get_type_name(wrapped_dict).c_str());
    }
    M_BaseObject* wrapped_keys = space->call_function(context, keys_impl, {});
    std::vector<M_BaseObject*> keys;
    space->unwrap_tuple(wrapped_keys, keys);

    std::unordered_map<std::string, M_BaseObject*> dict;
    for (auto& wrapped_key : keys) {
        std::string key = space->unwrap_str(wrapped_key);
        dict[key] = space->getitem(wrapped_dict, wrapped_key);
    }

    /* TODO: handle __slots__ */
    int add_dict = 1;

    M_StdTypeObject* cls =
        new (context) M_StdTypeObject(space, wrapped_type, name, bases, dict);
    cls->set_has_dict(add_dict > 0);
    cls->ready();
    return space->wrap(context, cls);
}

M_BaseObject* M_StdTypeObject::__call__(mtpython::vm::ThreadContext* context,
                                        const Arguments& args)
{
    Arguments arguments(args);
    ObjSpace* space = context->get_space();
    M_BaseObject* type = args.front();
    M_StdTypeObject* type_obj = static_cast<M_StdTypeObject*>(type);

    M_BaseObject* new_func = type_obj->lookup("__new__");
    if (!new_func) {
        throw InterpError::format(space, space->TypeError_type(),
                                  "cannot create '%s' instances",
                                  type_obj->name.c_str());
    }
    M_BaseObject* obj = space->call_args(context, new_func, arguments);

    type_obj = static_cast<M_StdTypeObject*>(space->type(obj));
    M_BaseObject* init_func = type_obj->lookup("__init__");
    if (init_func) {
        arguments.pop_front();
        arguments.prepend(obj);
        space->call_args(context, init_func, arguments);
    }

    return obj;
}

M_BaseObject* mtpython::objects::M_StdTypeObject::__subclasscheck__(
    vm::ThreadContext* context, M_BaseObject* self, M_BaseObject* sub)
{
    M_StdTypeObject* as_type = static_cast<M_StdTypeObject*>(self);
    M_StdTypeObject* sub_as_type = static_cast<M_StdTypeObject*>(sub);

    return context->get_space()->new_bool(sub_as_type->issubtype(as_type));
}

M_BaseObject* M_StdTypeObject::__repr__(mtpython::vm::ThreadContext* context,
                                        M_BaseObject* self)
{
    std::string str;
    str = "<class '";
    M_StdTypeObject* as_type = static_cast<M_StdTypeObject*>(self);
    str += as_type->name;
    str += "'>";

    return context->get_space()->wrap_str(context, str);
}

M_BaseObject* M_StdTypeObject::__mro__get(mtpython::vm::ThreadContext* context,
                                          M_BaseObject* self)
{
    M_StdTypeObject* as_type = static_cast<M_StdTypeObject*>(self);
    return context->get_space()->new_tuple(context, as_type->mro);
}

M_BaseObject*
M_StdTypeObject::__getattribute__(mtpython::vm::ThreadContext* context,
                                  M_BaseObject* obj, M_BaseObject* attr)
{
    ObjSpace* space = context->get_space();
    M_StdTypeObject* as_type = static_cast<M_StdTypeObject*>(obj);
    std::string name = space->unwrap_str(attr);
    M_BaseObject* descr = space->lookup(obj, name);

    if (descr) {
        M_BaseObject* getter = space->lookup(descr, "__get__");
        if (getter) {
            auto* type = space->type(obj);
            return space->get_and_call_function(context, getter,
                                                {descr, obj, type});
        }
    }

    M_BaseObject* value = as_type->lookup(name);
    if (value) {
        return space->get(value, space->wrap_None(), obj);
    }

    if (descr) return space->get(descr, obj);
    throw InterpError::format(space, space->AttributeError_type(),
                              "'%s' object has no attribute '%s'",
                              space->get_type_name(obj).c_str(), name.c_str());
}

void M_StdTypeObject::add_subclass(M_BaseObject* cls)
{
    /* TODO: use weak ref */
    for (auto p : subclasses) {
        if (p == cls) {
            return;
        }
    }

    subclasses.push_back(cls);
}

const std::vector<M_BaseObject*>& M_StdTypeObject::get_subclasses()
{
    return subclasses;
}

M_BaseObject*
M_StdTypeObject::__subclasses__(mtpython::vm::ThreadContext* context,
                                M_BaseObject* self)
{
    ObjSpace* space = context->get_space();

    self->lock();
    M_BaseObject* list = space->new_list(
        context, (static_cast<M_StdTypeObject*>(self))->get_subclasses());
    self->unlock();

    return list;
}
