#ifndef _STD_NONE_OBJECT_H_
#define _STD_NONE_OBJECT_H_

#include <string>
#include "objects/base_object.h"

namespace mtpython {
namespace objects {

class M_StdNoneObject : public M_BaseObject {
public:
	virtual bool i_is(ObjSpace* space, M_BaseObject* other) { return (other == space->wrap_None()); }
};

}
}

#endif /* _STD_NONE_OBJECT_H_ */
