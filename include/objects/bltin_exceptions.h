#ifndef _BLTIN_EXCEPTIONS_H_
#define _BLTIN_EXCEPTIONS_H_

#include "objects/obj_space.h"

namespace mtpython {
namespace objects {

class BaseException : public M_BaseObject {
public:
    static M_BaseObject* get_bltin_exception_type(ObjSpace* space,
                                                  const std::string& name);
};

} // namespace objects
} // namespace mtpython

#endif /* _BLTIN_EXCEPTIONS_H_ */
