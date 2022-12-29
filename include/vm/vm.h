#ifndef _VM_VM_H_
#define _VM_VM_H_

#include "vm/thread_context.h"
#include "objects/obj_space.h"
#include "gc/garbage_collector.h"

#include <memory>

namespace mtpython {

namespace vm {

class PyVM {
public:
    PyVM(mtpython::objects::ObjSpace* space, const std::string& executable);

    /* we have only one thread now */
    ThreadContext* get_main_thread() { return &main_thread_; }

    void init_bootstrap_path(const std::string& executable);
    void run_file(const std::string& filename);
    void run_toplevel(std::function<void()> f);

    void mark_roots();

private:
    objects::ObjSpace* space_;
    std::unique_ptr<gc::GarbageCollector> gc_;
    ThreadContext main_thread_;

    mtpython::interpreter::Module* init_main_module(ThreadContext* context);
    mtpython::interpreter::Code* compile_code(ThreadContext* context,
                                              const std::string& source,
                                              const std::string& filename,
                                              const std::string& mode);
    void run_eval_string(ThreadContext* context, const std::string& source,
                         const std::string& filename, bool eval);
};

} // namespace vm
} // namespace mtpython

#endif /* _VM_VM_H_ */
