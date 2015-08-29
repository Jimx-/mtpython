#include "objects/bltin_exceptions.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef BaseException_typedef("BaseException", {});

static mtpython::interpreter::Typedef Exception_typedef("Exception", { &BaseException_typedef },
	std::unordered_map<std::string, M_BaseObject*>{
});

static mtpython::interpreter::Typedef TypeError_typedef("TypeError", { &Exception_typedef }, {});
static mtpython::interpreter::Typedef StopIteration_typedef("StopIteration", { &Exception_typedef }, {});


static std::unordered_map<std::string, Typedef*> exception_typedefs{
	{ "BaseException", &BaseException_typedef },
	{ "Exception", &Exception_typedef },
	{ "TypeError", &TypeError_typedef },
	{ "StopIteration", &StopIteration_typedef },
};

M_BaseObject* BaseException::get_bltin_exception_type(ObjSpace* space, const std::string& name)
{
	Typedef* def = exception_typedefs[name];
	return space->get_typeobject(def);
}
