#ifndef _OBJ_SPACE_STD_H_
#define _OBJ_SPACE_STD_H_

#include "objects/obj_space.h"

namespace mtpython {
namespace objects {

/* General-purpose object space */
class StdObjSpace : public ObjSpace {
public:
	virtual M_BaseObject* wrap_int(int x);
	virtual M_BaseObject* wrap_int(std::string& x);
};

}
}

#endif
