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

class GetSetDescriptor : public objects::M_BaseObject {
private:
    PropertyGetter getter;
public:
    GetSetDescriptor(PropertyGetter getter)
    {
        this->getter = getter;
    }

    Typedef* get_typedef();

    static objects::M_BaseObject* __get__(vm::ThreadContext* context, objects::M_BaseObject* self, objects::M_BaseObject* obj, objects::M_BaseObject* cls);
};

}
}

#endif /* _INTERPRETER_DESCRIPTOR_H_ */