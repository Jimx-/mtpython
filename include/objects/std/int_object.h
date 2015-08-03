#ifndef _STD_INT_OBJECT_H_
#define _STD_INT_OBJECT_H_

#include <string>
#include "objects/base_object.h"

namespace mtpython {
namespace objects {

class M_StdIntObject : public M_BaseObject {
private:
	int intval;
public:
	M_StdIntObject(int x);
	M_StdIntObject(std::string& x);

	virtual void dbg_print();
};

}
}

#endif /* _STD_INT_OBJECT_H_ */
