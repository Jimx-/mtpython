#ifndef _STD_OBJECT_OBJECT_H_
#define _STD_OBJECT_OBJECT_H_

#include <string>
#include "objects/base_object.h"

namespace mtpython {
namespace objects {

class M_StdObjectObject : public M_BaseObject {
public:
	virtual interpreter::Typedef* get_typedef();

	static interpreter::Typedef* _object_typedef();
};

}
}

#endif /* _STD_OBJECT_OBJECT_H_ */
