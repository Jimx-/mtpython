#ifndef _INTERPRETER_CODE_H_
#define _INTERPRETER_CODE_H_

#include "objects/base_object.h"
#include <string>

namespace mtpython {
namespace interpreter {

class M_Code : mtpython::objects::M_BaseObject {
protected:
	std::string co_name;
public:
	M_Code(std::string& name) : co_name(name) { }
};

}
}

#endif /* _INTERPRETER_CODE_H_ */
