#include <string>
#include <iostream>
#include <functional>

#include "objects/std/obj_space_std.h"
#include "vm/vm.h"

int main(int argc, char* argv[])
{
    mtpython::objects::StdObjSpace space;
    mtpython::vm::PyVM vm(&space, argv[0]);

    std::function<void()> task;
    if (argc > 1) {
        std::string filename(argv[1]);
        task = [filename, &vm]() { vm.run_file(filename); };
    }
    vm.run_toplevel(task);

    return 0;
}
