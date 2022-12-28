#ifndef _VM_NATIVE_THREAD_H_
#define _VM_NATIVE_THREAD_H_

#include <cstddef>
#include <tuple>

namespace mtpython {

namespace vm {

class ThreadContext;

class NativeThread {
public:
    NativeThread(ThreadContext* thread) : thread_(thread) {}

    virtual void init_main() = 0;
    virtual void create() = 0;
    virtual void destory() = 0;

    static ThreadContext* current_thread() { return current_thread_; }

    struct MachineStack {
        void* stack_start;
        void* stack_end;
        size_t max_size;
    };

    const MachineStack& get_stack() const { return stack_; }

    virtual void save_stack_end() = 0;

protected:
    static thread_local ThreadContext* current_thread_;

    ThreadContext* thread_;
    MachineStack stack_;
};

} // namespace vm

} // namespace mtpython

#endif
