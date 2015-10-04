#ifndef _POSIXMODULE_H_
#define _POSIXMODULE_H_

#include "interpreter/module.h"
#include "objects/obj_space.h"

namespace mtpython {
namespace modules {

class PosixModule : public interpreter::BuiltinModule {
public:
	PosixModule(objects::ObjSpace* space, objects::M_BaseObject* name);
};

}
}

#endif /* _POSIXMODULE_H_ */
