#ifndef _STD_STR_OBJECT_H_
#define _STD_STR_OBJECT_H_

#include <string>
#include "objects/base_object.h"

namespace mtpython {
namespace objects {

class M_AbstractStdStrObject : public M_BaseObject {
};

class M_StdStrObject : public M_AbstractStdStrObject {
private:
	std::string value;
public:
	M_StdStrObject(std::string& s);

	static M_BaseObject* __repr__(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* self);

	static interpreter::Typedef* _str_typedef();
	virtual interpreter::Typedef* get_typedef();

	virtual void dbg_print();
};

}
}

#endif /* _STD_STR_OBJECT_H_ */
