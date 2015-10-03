#ifndef _FILEIO_H_
#define _FILEIO_H_

#include "objects/obj_space.h"
#include "modules/_io/iobase.h"

namespace mtpython {
namespace modules {

class M_FileIO : public M_RawIOBase {	
private:
	M_BaseObject* name;
	int fd;
	bool closefd;
public:
	M_FileIO(mtpython::objects::ObjSpace* space) : M_RawIOBase(space) { }

	interpreter::Typedef* get_typedef();

	static objects::M_BaseObject* __new__(vm::ThreadContext* context, objects::M_BaseObject* type, objects::M_BaseObject* args, objects::M_BaseObject* kwargs);
	static objects::M_BaseObject* __init__(vm::ThreadContext* context, objects::M_BaseObject* self, objects::M_BaseObject* args, objects::M_BaseObject* kwargs);

	static objects::M_BaseObject* name_get(vm::ThreadContext* context, objects::M_BaseObject* self);
	static void name_set(vm::ThreadContext* context, objects::M_BaseObject* obj, objects::M_BaseObject* value);
};


}
}

#endif /* _FILEIO_H_ */
