#include <string>
#include <iostream>

#include "objects/std/obj_space_std.h"
#include "vm/vm.h"

int main(int argc, char * argv[])
{
	mtpython::objects::StdObjSpace space;
	mtpython::vm::PyVM vm(&space, argv[0]);

	vm.run_file("/home/jimx/a.py");

	return 0;
}
