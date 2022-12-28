#ifndef _VM_NATIVE_THREAD_POSIX_H_
#define _VM_NATIVE_THREAD_POSIX_H_

#include "vm/native_thread.h"

#include <pthread.h>

namespace mtpython {

namespace vm {

class NativeThreadPOSIX : public NativeThread {
public:
    NativeThreadPOSIX(ThreadContext* thread);

    virtual void init_main();
    virtual void create();
    virtual void destory();

    virtual void save_stack_end();

private:
    pthread_t thread_id_;

    int get_pthread_stack(void*& addr, size_t& size);
};

} // namespace vm

} // namespace mtpython

#endif
