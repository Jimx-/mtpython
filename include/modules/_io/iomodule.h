#ifndef _IOMODULE_H_
#define _IOMODULE_H_

#include "interpreter/module.h"
#include "objects/obj_space.h"

namespace mtpython {
namespace modules {

#define DEFAULT_BUFFER_SIZE 8192

class IOModule : public mtpython::interpreter::BuiltinModule {
public:
    IOModule(mtpython::objects::ObjSpace* space,
             mtpython::objects::M_BaseObject* name);
};

} // namespace modules
} // namespace mtpython

#endif /* _IOMODULE_H_ */
