#ifndef _OBJ_SPACE_STD_H_
#define _OBJ_SPACE_STD_H_

#include "objects/obj_space.h"
#include <unordered_map>

namespace mtpython {
namespace objects {

/* General-purpose object space */
class StdObjSpace : public ObjSpace {
private:
	std::unordered_map<std::string, M_BaseObject*> builtin_types;
	
public:
	StdObjSpace();

	virtual M_BaseObject* wrap(M_BaseObject* obj) { return obj->bind_space(this); }

	virtual M_BaseObject* wrap_int(int x);
	virtual M_BaseObject* wrap_int(std::string& x);
};

}
}

#endif
