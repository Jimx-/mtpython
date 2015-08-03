#include <string.h>
#include <iostream>
#include "objects/std/int_object.h"

using namespace mtpython::objects;

M_StdIntObject::M_StdIntObject(int x)
{
	intval = x;
}

M_StdIntObject::M_StdIntObject(std::string& x)
{
	intval = std::stoi(x, nullptr, 0);
}

void M_StdIntObject::dbg_print()
{
	std::cout << intval;
}
