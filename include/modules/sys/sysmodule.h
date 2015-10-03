#ifndef _SYSMODULE_H_
#define _SYSMODULE_H_

#include "interpreter/module.h"
#include "objects/obj_space.h"

namespace mtpython {
	namespace modules {

		class SysModule : public mtpython::interpreter::BuiltinModule {
		public:
			SysModule(mtpython::vm::ThreadContext* context, mtpython::objects::M_BaseObject* name);
			void initstdio(mtpython::vm::ThreadContext* context, bool unbuffered = false);
		};

	}
}

#endif /* _SYSMODULE_H_ */
