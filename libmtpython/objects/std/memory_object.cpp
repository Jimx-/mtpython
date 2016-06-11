#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "objects/std/memory_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef memoryview_typedef("memoryview", {
});

M_StdMemoryViewObject::M_StdMemoryViewObject()
{
}

Typedef* M_StdMemoryViewObject::_memoryview_typedef()
{
    return &memoryview_typedef;
}

Typedef* M_StdMemoryViewObject::get_typedef()
{
    return &memoryview_typedef;
}

