#ifndef _STD_BOOL_OBJECT_H_
#define _STD_BOOL_OBJECT_H_

#include <string>
#include "objects/std/int_object.h"

namespace mtpython {
namespace objects {

#define M_STDBOOLOBJECT(obj) (dynamic_cast<M_StdBoolObject*>(obj))

class M_StdBoolObject : public M_StdIntObject {
public:
	M_StdBoolObject(bool x);

	virtual bool i_is(ObjSpace* space, M_BaseObject* other);

	static M_BaseObject* __repr__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self);
	static M_BaseObject* __str__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self);
	static M_BaseObject* __bool__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self);

	static interpreter::Typedef* _bool_typedef();
	virtual interpreter::Typedef* get_typedef();
};

}
}

#endif /* _STD_BOOL_OBJECT_H_ */
