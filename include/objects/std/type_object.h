#ifndef _STD_TYPE_OBJECT_H_
#define _STD_TYPE_OBJECT_H_

#include <string>
#include <vector>
#include <unordered_map>

#include "objects/base_object.h"
#include "objects/space_cache.h"
#include "interpreter/arguments.h"
#include "vm/vm.h"
#include "gc/garbage_collector.h"

namespace mtpython {
namespace objects {

class M_StdTypeObject : public M_BaseObject {
private:
    ObjSpace* space;
    std::string name;
    std::vector<M_BaseObject*> bases;
    std::unordered_map<std::string, M_BaseObject*> dict;
    M_BaseObject* wrapped_dict;
    std::vector<M_BaseObject*> mro;
    std::vector<M_BaseObject*> subclasses;
    bool _has_dict;
    M_BaseObject* cls;

    void init_mro();

    void ready();

public:
    M_StdTypeObject(ObjSpace* space, M_BaseObject* cls, std::string& name,
                    const std::vector<M_BaseObject*>& bases,
                    const std::unordered_map<std::string, M_BaseObject*>& dict);

    std::string get_name() { return name; }
    bool has_dict() { return _has_dict; }
    void set_has_dict(bool has_dict) { _has_dict = has_dict; }
    M_BaseObject* get_dict(ObjSpace* space);

    M_BaseObject* get_class(ObjSpace* space) { return cls; }
    void set_class(ObjSpace* space, M_BaseObject* type) { cls = type; }

    interpreter::Typedef* get_typedef();
    static interpreter::Typedef* _type_typedef();

    M_BaseObject* get_dict_value(ObjSpace* space, const std::string& attr);
    bool set_dict_value(ObjSpace* space, const std::string& attr,
                        M_BaseObject* value);

    M_BaseObject* lookup(const std::string& name);
    M_BaseObject* lookup_starting_at(M_BaseObject* start,
                                     const std::string& name);
    M_BaseObject* lookup_cls(const std::string& attr, M_BaseObject*& cls);

    void add_subclass(M_BaseObject* cls);
    const std::vector<M_BaseObject*>& get_subclasses();
    bool issubtype(M_BaseObject* type);

    virtual void mark_children(gc::GarbageCollector* gc);

    static M_BaseObject* __new__(vm::ThreadContext* context,
                                 const interpreter::Arguments& args);

    static M_BaseObject* __repr__(vm::ThreadContext* context,
                                  M_BaseObject* self);
    static M_BaseObject* __mro__get(vm::ThreadContext* context,
                                    M_BaseObject* self);

    static M_BaseObject* __call__(vm::ThreadContext* context,
                                  const interpreter::Arguments& args);
    static M_BaseObject* __subclasscheck__(vm::ThreadContext* context,
                                           M_BaseObject* self,
                                           M_BaseObject* sub);
    static M_BaseObject* __getattribute__(mtpython::vm::ThreadContext* context,
                                          M_BaseObject* obj,
                                          M_BaseObject* attr);
    static M_BaseObject* __subclasses__(mtpython::vm::ThreadContext* context,
                                        M_BaseObject* self);
};

class StdTypedefCache : public TypedefCache {
public:
    StdTypedefCache(ObjSpace* space) : TypedefCache(space) {}

    M_BaseObject* build(interpreter::Typedef* key);
};

} // namespace objects
} // namespace mtpython

#endif /* _STD_TYPE_OBJECT_H_ */
