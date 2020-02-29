#ifndef _STD_MEMORY_OBJECT_H_
#define _STD_MEMORY_OBJECT_H_

#include "objects/base_object.h"

namespace mtpython {
namespace objects {

#define M_STDMEMORYOBJECT(obj) (static_cast<M_StdMemoryObject*>(obj))

class M_StdMemoryViewObject : public M_BaseObject {
private:
public:
    M_StdMemoryViewObject();

    static interpreter::Typedef* _memoryview_typedef();
    interpreter::Typedef* get_typedef();
};

} // namespace objects
} // namespace mtpython

#endif /* _STD_MEMORY_OBJECT_H_ */
