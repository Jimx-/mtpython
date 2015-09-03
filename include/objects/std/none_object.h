#ifndef _STD_NONE_OBJECT_H_
#define _STD_NONE_OBJECT_H_

#include <string>
#include "objects/base_object.h"
#include "interpreter/typedef.h"
#include "vm/vm.h"

namespace mtpython {
namespace objects {

class M_StdNoneObject : public M_BaseObject {
public:
	virtual interpreter::Typedef* get_typedef();

	virtual bool i_is(ObjSpace* space, M_BaseObject* other) { return (other == space->wrap_None()); }

	static M_BaseObject* __repr__(vm::ThreadContext* context, M_BaseObject* self);
};

}
}

#endif /* _STD_NONE_OBJECT_H_ */
