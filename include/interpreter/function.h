#ifndef _INTERPRETER_FUNCTION_H_
#define _INTERPRETER_FUNCTION_H_

#include "objects/base_object.h"
#include "objects/obj_space.h"
#include "interpreter/code.h"
#include "interpreter/arguments.h"
#include "gc/garbage_collector.h"
#include "vm/vm.h"
#include <string>

namespace mtpython {
namespace interpreter {

class Function : public objects::M_BaseObject {
private:
    objects::ObjSpace* space;
    std::string name;
    Code* code;
    objects::M_BaseObject* func_globals;
    objects::M_BaseObject* func_dict;
    objects::M_BaseObject* doc;
    std::vector<objects::M_BaseObject*> defaults;
    std::vector<objects::M_BaseObject*> closure;

public:
    Function(objects::ObjSpace* space, Code* code,
             objects::M_BaseObject* globals = nullptr)
        : Function(space, code, {}, globals)
    {}

    Function(objects::ObjSpace* space, Code* code,
             const std::vector<objects::M_BaseObject*>& defaults,
             objects::M_BaseObject* globals = nullptr);
    Typedef* get_typedef();

    Code* get_code() { return code; }
    objects::M_BaseObject* get_dict(objects::ObjSpace* space);

    objects::M_BaseObject* get_globals() { return func_globals; }
    std::string& get_name() { return name; }
    const std::vector<objects::M_BaseObject*>& get_defaults()
    {
        return defaults;
    }
    const std::vector<objects::M_BaseObject*>& get_closure() { return closure; }
    void set_closure(const std::vector<objects::M_BaseObject*>& closure)
    {
        this->closure = closure;
    }

    objects::M_BaseObject* call_args(vm::ThreadContext* context,
                                     Arguments& args);
    objects::M_BaseObject* call_obj_args(vm::ThreadContext* context,
                                         objects::M_BaseObject* obj,
                                         Arguments& args);

    virtual void mark_children(gc::GarbageCollector* gc);

    static objects::M_BaseObject* __get__(vm::ThreadContext* context,
                                          objects::M_BaseObject* self,
                                          objects::M_BaseObject* obj,
                                          objects::M_BaseObject* type);
    static objects::M_BaseObject* __doc__get(vm::ThreadContext* context,
                                             objects::M_BaseObject* self);
    static void __doc__set(vm::ThreadContext* context,
                           objects::M_BaseObject* obj,
                           objects::M_BaseObject* value);
};

/* A method is a function bound to an instance */
class Method : public objects::M_BaseObject {
private:
    objects::ObjSpace* space;
    objects::M_BaseObject* func;
    objects::M_BaseObject* instance;

public:
    Method(objects::ObjSpace* space, objects::M_BaseObject* func,
           objects::M_BaseObject* instance)
        : space(space), func(func), instance(instance)
    {}

    objects::M_BaseObject* call_args(vm::ThreadContext* context,
                                     Arguments& args);

    virtual void mark_children(gc::GarbageCollector* gc)
    {
        gc->mark_object(func);
        gc->mark_object(instance);
    }
};

class StaticMethod : public objects::M_BaseObject {
private:
    objects::M_BaseObject* func;

public:
    StaticMethod(objects::M_BaseObject* func) : func(func) {}
    Typedef* get_typedef();
    static Typedef* _staticmethod_typedef();

    virtual void mark_children(gc::GarbageCollector* gc)
    {
        gc->mark_object(func);
    }

    static objects::M_BaseObject* __new__(vm::ThreadContext* context,
                                          objects::M_BaseObject* type,
                                          objects::M_BaseObject* func);
    static objects::M_BaseObject* __get__(vm::ThreadContext* context,
                                          const Arguments& args);
};

class ClassMethod : public objects::M_BaseObject {
private:
    objects::M_BaseObject* func;

public:
    ClassMethod(objects::M_BaseObject* func) : func(func) {}
    Typedef* get_typedef();
    static Typedef* _classmethod_typedef();

    virtual void mark_children(gc::GarbageCollector* gc)
    {
        gc->mark_object(func);
    }

    static objects::M_BaseObject* __new__(vm::ThreadContext* context,
                                          objects::M_BaseObject* type,
                                          objects::M_BaseObject* func);
    static objects::M_BaseObject* __get__(vm::ThreadContext* context,
                                          const Arguments& args);
};

} // namespace interpreter
} // namespace mtpython

#endif /* _INTERPRETER_FUNCTION_H_ */
