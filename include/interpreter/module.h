#ifndef _INTERPRETER_MODULE_H_
#define _INTERPRETER_MODULE_H_

#include "objects/base_object.h"
#include "objects/obj_space.h"
#include "interpreter/code.h"
#include "interpreter/arguments.h"
#include "vm/vm.h"
#include <string>

namespace mtpython {
namespace interpreter {

class Module : public objects::M_BaseObject {
private:
	objects::ObjSpace* space;
	objects::M_BaseObject* name;
	objects::M_BaseObject* dict;
public:
	Module(objects::ObjSpace* space, objects::M_BaseObject* name, objects::M_BaseObject* dict);
};

}
}

#endif /* _INTERPRETER_MODULE_H_ */
