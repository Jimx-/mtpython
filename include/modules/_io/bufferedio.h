#ifndef _BUFFEREDIO_H_
#define _BUFFEREDIO_H_

#include "objects/obj_space.h"
#include "modules/_io/iobase.h"

namespace mtpython {
namespace modules {

class M_BufferedIOBase : public M_IOBase {
public:
	M_BufferedIOBase(objects::ObjSpace* space) : M_IOBase(space) { }

	interpreter::Typedef* get_typedef();
};

class BufferedBase : public M_BufferedIOBase {
protected:
	objects::M_BaseObject* raw;
public:
	BufferedBase(objects::ObjSpace* space) : M_BufferedIOBase(space) { }

	static objects::M_BaseObject* name_get(vm::ThreadContext* context, objects::M_BaseObject* self);
};

class M_BufferedReader : public BufferedBase {
public:
	M_BufferedReader(objects::ObjSpace* space) : BufferedBase(space) { }

	interpreter::Typedef* get_typedef();

	static objects::M_BaseObject* __new__(vm::ThreadContext* context, objects::M_BaseObject* type, objects::M_BaseObject* args, objects::M_BaseObject* kwargs);
	static objects::M_BaseObject* __init__(vm::ThreadContext* context, objects::M_BaseObject* self, objects::M_BaseObject* args, objects::M_BaseObject* kwargs);
};

}
}

#endif /* _BUFFEREDIO_H_ */
