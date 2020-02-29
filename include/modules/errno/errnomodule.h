#ifndef _ERRNOMODULE_H_
#define _ERRNOMODULE_H_

#include "interpreter/module.h"
#include "objects/obj_space.h"

namespace mtpython {
namespace modules {

class ErrnoModule : public interpreter::BuiltinModule {
public:
    ErrnoModule(objects::ObjSpace* space, objects::M_BaseObject* name);
};

} // namespace modules
} // namespace mtpython

#endif /* _ERRNOMODULE_H_ */
