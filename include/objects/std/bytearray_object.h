#ifndef _STD_BYTEARRAY_OBJECT_H_
#define _STD_BYTEARRAY_OBJECT_H_

#include <string>
#include "objects/base_object.h"

namespace mtpython {
namespace objects {

#define M_STDBYTEARRAYOBJECT(obj) (static_cast<M_StdByteArrayObject*>(obj))

class M_StdByteArrayObject : public M_BaseObject {
private:
    std::string array;

public:
    M_StdByteArrayObject();

    static M_BaseObject* __iter__(mtpython::vm::ThreadContext* context,
                                  M_BaseObject* self);

    static interpreter::Typedef* _bytearray_typedef();
    interpreter::Typedef* get_typedef();
};

} // namespace objects
} // namespace mtpython

#endif /* _STD_BYTEARRAY_OBJECT_H_ */
