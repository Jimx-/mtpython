#ifndef __WEAKREF_MODULE_H_
#define __WEAKREF_MODULE_H_

#include "interpreter/module.h"
#include "objects/obj_space.h"

namespace mtpython {
namespace modules {

class WeakrefModule : public mtpython::interpreter::BuiltinModule {
public:
    WeakrefModule(mtpython::objects::ObjSpace* space,
                  mtpython::objects::M_BaseObject* name);
};

} // namespace modules
} // namespace mtpython

#endif /* __WEAKREF_MODULE_H_ */
