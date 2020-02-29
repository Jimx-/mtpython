#include "interpreter/typedef.h"
#include "interpreter/gateway.h"
#include "objects/std/bytes_object.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static mtpython::interpreter::Typedef bytes_typedef("bytes", {});

M_StdBytesObject::M_StdBytesObject() {}

Typedef* M_StdBytesObject::_bytes_typedef() { return &bytes_typedef; }

Typedef* M_StdBytesObject::get_typedef() { return &bytes_typedef; }
