#ifndef _OBJ_SPACE_H_
#define _OBJ_SPACE_H_

namespace mtpython {
namespace vm {
class PyVM;
}
}

#include <string>
#include "objects/base_object.h"

namespace mtpython {
namespace objects {

class ObjSpace {
protected:
	mtpython::vm::PyVM* vm;
public:
	
	void set_vm(mtpython::vm::PyVM* vm) { this->vm = vm; }

	virtual M_BaseObject* wrap_int(int x) { return nullptr; }
	virtual M_BaseObject* wrap_int(std::string& x) { return nullptr; }
};

}
}

#endif
