#ifndef _BLTINMODULE_H_
#define _BLTINMODULE_H_

#include "interpreter/module.h"
#include "objects/obj_space.h"

namespace mtpython {
	namespace modules {

		class BuiltinsModule : public mtpython::interpreter::Module {
		public:
			BuiltinsModule(mtpython::objects::ObjSpace* space, mtpython::objects::M_BaseObject* name);
		};

	}
}

#endif /* _BLTINMODULE_H_ */
