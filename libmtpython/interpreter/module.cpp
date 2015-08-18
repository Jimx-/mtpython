#include "interpreter/module.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;

Module::Module(ObjSpace* space, M_BaseObject* name, M_BaseObject* dict)
{
	this->space = space;
	this->name = name;
	this->dict = dict;
}
