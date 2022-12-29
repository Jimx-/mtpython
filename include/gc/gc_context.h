#ifndef _GC_GC_CONTEXT_H_
#define _GC_GC_CONTEXT_H_

#include "vm/thread_context.h"

namespace mtpython {

namespace gc {

class GCContext {
public:
    GCContext(vm::ThreadContext* thread) : thread_(thread) {}
    virtual ~GCContext() {}

protected:
    vm::ThreadContext* thread_;
};

} // namespace gc

} // namespace mtpython

#endif
