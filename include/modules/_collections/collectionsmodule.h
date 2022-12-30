#ifndef __COLLECTIONS_MODULE_H_
#define __COLLECTIONS_MODULE_H_

#include "interpreter/module.h"
#include "objects/obj_space.h"

namespace mtpython {
namespace modules {

class CollectionsModule : public mtpython::interpreter::BuiltinModule {
public:
    CollectionsModule(mtpython::objects::ObjSpace* space,
                      mtpython::objects::M_BaseObject* name);
};

} // namespace modules
} // namespace mtpython

#endif
