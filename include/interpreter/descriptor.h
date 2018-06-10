#ifndef _INTERPRETER_DESCRIPTOR_H_
#define _INTERPRETER_DESCRIPTOR_H_

#include "objects/base_object.h"
#include "interpreter/arguments.h"
#include "typedef.h"
#include "vm/vm.h"
#include <string>

namespace mtpython {
namespace interpreter {

typedef objects::M_BaseObject* (*PropertyGetter)(vm::ThreadContext* context, objects::M_BaseObject* obj);
typedef void (*PropertySetter)(vm::ThreadContext* context, objects::M_BaseObject* obj, objects::M_BaseObject* value);

class GetSetDescriptor : public objects::M_BaseObject {
private:
    PropertyGetter getter;
    PropertySetter setter;
public:
    GetSetDescriptor(PropertyGetter getter, PropertySetter setter = nullptr)
    {
        this->getter = getter;
        this->setter = setter;
    }

    void* operator new(size_t size) { return ::operator new(size); }
    Typedef* get_typedef();

    static objects::M_BaseObject* __get__(vm::ThreadContext* context, objects::M_BaseObject* self, objects::M_BaseObject* obj, objects::M_BaseObject* cls);
    static objects::M_BaseObject* __set__(vm::ThreadContext* context, objects::M_BaseObject* self, objects::M_BaseObject* obj, objects::M_BaseObject* value);
};

}
}

#endif /* _INTERPRETER_DESCRIPTOR_H_ */