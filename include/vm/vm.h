#ifndef _VM_VM_H_
#define _VM_VM_H_

#include "vm/thread_context.h"

namespace mtpython {

namespace vm {

class PyVM {
private:
    objects::ObjSpace* space;
    gc::Heap* heap;

    ThreadContext main_thread;

    mtpython::interpreter::Module* init_main_module(ThreadContext* context);
    mtpython::interpreter::Code* compile_code(ThreadContext* context,
                                              const std::string& source,
                                              const std::string& filename,
                                              const std::string& mode);
    void run_eval_string(ThreadContext* context, const std::string& source,
                         const std::string& filename, bool eval);

public:
    PyVM(mtpython::objects::ObjSpace* space, const std::string& executable);

    /* we have only one thread now */
    ThreadContext* current_thread() { return &main_thread; }
    gc::Heap* get_heap() { return heap; }

    void init_bootstrap_path(const std::string& executable);
    void run_file(const std::string& filename);
    void run_toplevel(std::function<void()> f);
};

} // namespace vm
} // namespace mtpython

#endif /* _VM_VM_H_ */
