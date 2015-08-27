#ifndef _STD_STR_OBJECT_H_
#define _STD_STR_OBJECT_H_

#include <string>
#include "objects/base_object.h"

namespace mtpython {
namespace objects {

#define M_STDSTROBJECT(obj) (dynamic_cast<M_StdStrObject*>(obj))
	
class M_AbstractStdStrObject : public M_BaseObject {
};

class M_StdStrObject : public M_AbstractStdStrObject {
private:
	std::string value;
public:
	M_StdStrObject(const std::string& s);

	virtual bool i_is(ObjSpace* space, M_BaseObject* other) { return (M_STDSTROBJECT(other) != nullptr); }

	static M_BaseObject* __repr__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self);
	static M_BaseObject* __hash__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self);
	static M_BaseObject* __eq__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self, mtpython::objects::M_BaseObject* other);

	static interpreter::Typedef* _str_typedef();
	virtual interpreter::Typedef* get_typedef();

	virtual std::string to_string(ObjSpace* space) { return value; }

	virtual void dbg_print();
};

}
}

#endif /* _STD_STR_OBJECT_H_ */
