#ifndef _BASE_OBJECT_H_
#define _BASE_OBJECT_H_

#include "base_object.h"

namespace mtpython {

namespace interpreter {
class Typedef;
}

namespace objects {

class ObjSpace;

/* Base of all MTPython objects */
class M_BaseObject {
public:
	virtual M_BaseObject* get_class(ObjSpace* space) { return nullptr; }
	virtual interpreter::Typedef* get_typedef() { return nullptr; }

	virtual M_BaseObject* bind_space(ObjSpace* space) { return this; }
	
	virtual void dbg_print() { }	/* for debug purpose */
};

}
}

#endif
