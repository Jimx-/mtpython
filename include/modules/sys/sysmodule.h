#ifndef _SYSMODULE_H_
#define _SYSMODULE_H_

#include "interpreter/module.h"
#include "objects/obj_space.h"
#include "objects/base_object.h"

#include <string>

namespace mtpython {
namespace modules {

class SysModule : public mtpython::interpreter::BuiltinModule {
private:
	static std::string filesystem_encoding;
public:
	SysModule(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* name);
	void initstdio(mtpython::vm::ThreadContext* context, bool unbuffered = false);
	void initfsencoding(mtpython::vm::ThreadContext* context);

	static objects::M_BaseObject* getfilesystemencoding(mtpython::vm::ThreadContext* context);
};

}
}

#endif /* _SYSMODULE_H_ */
