#ifndef _STD_FRAME_H_
#define _STD_FRAME_H_

#include "interpreter/pyframe.h"

namespace mtpython {
namespace objects {

class StdFrame : public interpreter::PyFrame {
public:
    StdFrame(vm::ThreadContext* context, interpreter::Code* code,
             mtpython::objects::M_BaseObject* globals,
             mtpython::objects::M_BaseObject* outer)
        : interpreter::PyFrame(context, code, globals, outer)
    {}
};

} // namespace objects
} // namespace mtpython

#endif /* _STD_FRAME_H_ */
