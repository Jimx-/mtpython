#include <string>
#include <iostream>

#include "objects/std/obj_space_std.h"
#include "vm/vm.h"

int main(int argc, char * argv[])
{
	mtpython::objects::StdObjSpace space;
	mtpython::vm::PyVM vm(&space, argv[0]);

	std::string path("/home/jimx/a.py");
	vm.run_file(path);

	return 0;
}
