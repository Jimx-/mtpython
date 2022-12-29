#include <string>
#include <iostream>
#include <functional>

#include "objects/std/obj_space_std.h"
#include "vm/vm.h"

#include "spdlog/cfg/env.h"

int main(int argc, char* argv[])
{
    spdlog::cfg::load_env_levels();

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
