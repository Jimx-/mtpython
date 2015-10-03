#ifndef _IOBASE_H_
#define _IOBASE_H_

#include "objects/obj_space.h"

namespace mtpython {
namespace modules {

class M_IOBase : public mtpython::objects::M_BaseObject {
private:
	mtpython::objects::M_BaseObject* dict;	
public:
	M_IOBase(mtpython::objects::ObjSpace* space);

	objects::M_BaseObject* get_dict(objects::ObjSpace* space) { return dict; }
};

class M_RawIOBase : public M_IOBase {
public:
	M_RawIOBase(mtpython::objects::ObjSpace* space) : M_IOBase(space) { }
};

}
}

#endif /* _IOBASE_H_ */
