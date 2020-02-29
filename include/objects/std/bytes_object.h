#ifndef _STD_BYTES_OBJECT_H_
#define _STD_BYTES_OBJECT_H_

#include <string>
#include "objects/base_object.h"

namespace mtpython {
namespace objects {

#define M_STDBYTESOBJECT(obj) (static_cast<M_StdBytesObject*>(obj))

class M_StdBytesObject : public M_BaseObject {
private:
public:
    M_StdBytesObject();

    static interpreter::Typedef* _bytes_typedef();
    interpreter::Typedef* get_typedef();
};

} // namespace objects
} // namespace mtpython

#endif /* _STD_BYTES_OBJECT_H_ */
