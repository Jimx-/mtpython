#include "vm/native_thread_posix.h"

#include <iostream>

namespace mtpython {

namespace vm {

NativeThreadPOSIX::NativeThreadPOSIX(ThreadContext* thread)
    : NativeThread(thread)
{}

int NativeThreadPOSIX::get_pthread_stack(void*& addr, size_t& size)
{
    pthread_attr_t attr;
    size_t guard;
    int err;

    err = pthread_getattr_np(pthread_self(), &attr);
    if (err) return err;

    err = pthread_attr_getstack(&attr, &addr, &size);
    if (err) return err;

    err = pthread_attr_getguardsize(&attr, &guard);
    if (err) return err;

    addr = (char*)addr + size;
    size -= guard;

    pthread_attr_destroy(&attr);

    return 0;
}

void NativeThreadPOSIX::init_main()
{
    void* stackaddr;
    size_t size;

    thread_id_ = pthread_self();

    if (get_pthread_stack(stackaddr, size) == 0) {
        stack_.stack_start = stackaddr;
        stack_.max_size = size;
    }

    current_thread_ = thread_;
}

void NativeThreadPOSIX::create() {}

void NativeThreadPOSIX::destory() {}

void NativeThreadPOSIX::save_stack_end()
{
#if defined(__x86_64__) && !defined(_ILP32) && defined(__GNUC__)
    __asm__ __volatile__("movq\t%%rsp, %0" : "=r"(stack_.stack_end));
#endif
}

} // namespace vm

} // namespace mtpython
