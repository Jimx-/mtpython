#ifndef _FILEIO_H_
#define _FILEIO_H_

#include "objects/obj_space.h"
#include "modules/_io/iobase.h"

namespace mtpython {
namespace modules {

class M_FileIO : public M_RawIOBase {	
private:
	int fd;
	bool closefd;
public:
	M_FileIO(mtpython::objects::ObjSpace* space) : M_RawIOBase(space) { }

	interpreter::Typedef* get_typedef();

	static objects::M_BaseObject* __new__(vm::ThreadContext* context, objects::M_BaseObject* type, objects::M_BaseObject* args, objects::M_BaseObject* kwargs);
	static objects::M_BaseObject* __init__(vm::ThreadContext* context, objects::M_BaseObject* self, objects::M_BaseObject* args, objects::M_BaseObject* kwargs);
};


}
}

#endif /* _FILEIO_H_ */
