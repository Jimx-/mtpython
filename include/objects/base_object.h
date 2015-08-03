#ifndef _BASE_OBJECT_H_
#define _BASE_OBJECT_H_

#include "base_object.h"

namespace mtpython {
namespace objects {

/* Base of all MTPython objects */
class M_BaseObject {
public:

	virtual void dbg_print() { }	/* for debug purpose */
};

}
}

#endif
