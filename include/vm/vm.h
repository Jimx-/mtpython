#ifndef _VM_H_
#define _VM_H_

#include "objects/obj_space.h"

namespace mtpython {
namespace vm {

class PyVM {
private:
	mtpython::objects::ObjSpace* space;
public:
	PyVM(mtpython::objects::ObjSpace* space);

	void run_file(std::string& filename);
};

}
}

#endif /* _VM_H_ */
