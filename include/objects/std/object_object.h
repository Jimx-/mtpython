#ifndef _STD_OBJECT_OBJECT_H_
#define _STD_OBJECT_OBJECT_H_

#include <string>
#include "objects/base_object.h"
#include "vm/vm.h"

namespace mtpython {
namespace objects {

class M_StdObjectObject : public M_BaseObject {
public:
	virtual interpreter::Typedef* get_typedef();
	static interpreter::Typedef* _object_typedef();

	static M_BaseObject* __getattribute__(vm::ThreadContext* context, M_BaseObject* obj, M_BaseObject* attr);
	static M_BaseObject* __setattr__(vm::ThreadContext* context, M_BaseObject* obj, M_BaseObject* attr, M_BaseObject* value);
};

}
}

#endif /* _STD_OBJECT_OBJECT_H_ */
