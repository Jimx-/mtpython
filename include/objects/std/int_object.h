#ifndef _STD_INT_OBJECT_H_
#define _STD_INT_OBJECT_H_

#include <string>
#include <sstream>

#include "objects/base_object.h"

namespace mtpython {
namespace objects {

#define M_STDINTOBJECT(obj) (dynamic_cast<M_StdIntObject*>(obj))

class M_StdIntObject : public M_BaseObject {
protected:
	int intval;
public:
	M_StdIntObject(int x);
	M_StdIntObject(const std::string& x);

	static M_BaseObject* __repr__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self);
	static M_BaseObject* __str__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self);
	static M_BaseObject* __bool__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self);
	static M_BaseObject* __add__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self, mtpython::objects::M_BaseObject* other);
	static M_BaseObject* __sub__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self, mtpython::objects::M_BaseObject* other);
	static M_BaseObject* __mul__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self, mtpython::objects::M_BaseObject* other);

	static M_BaseObject* __eq__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self, mtpython::objects::M_BaseObject* other);
	static M_BaseObject* __ne__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self, mtpython::objects::M_BaseObject* other);
	static M_BaseObject* __lt__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self, mtpython::objects::M_BaseObject* other);
	static M_BaseObject* __le__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self, mtpython::objects::M_BaseObject* other);
	static M_BaseObject* __gt__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self, mtpython::objects::M_BaseObject* other);
	static M_BaseObject* __ge__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self, mtpython::objects::M_BaseObject* other);

	static M_BaseObject* __abs__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self);
	static M_BaseObject* __neg__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self);

	static interpreter::Typedef* _int_typedef();
	virtual interpreter::Typedef* get_typedef();

	virtual int to_int(ObjSpace* space, bool allow_conversion) { return intval; }
	virtual std::string to_string(ObjSpace* space)
	{
		std::ostringstream stm;
		stm << intval;
		return stm.str();
	}

	virtual void dbg_print();
};

}
}

#endif /* _STD_INT_OBJECT_H_ */
