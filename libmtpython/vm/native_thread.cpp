#include "vm/native_thread.h"
#include "vm/thread_context.h"

namespace mtpython {

namespace vm {

thread_local ThreadContext* NativeThread::current_thread_ = nullptr;

}

} // namespace mtpython
