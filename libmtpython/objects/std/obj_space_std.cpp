#include <string.h>
#include "interpreter/code.h"
#include "objects/base_object.h"
#include "objects/std/obj_space_std.h"
#include "objects/std/type_object.h"
#include "objects/std/int_object.h"
#include "objects/std/str_object.h"
#include "objects/std/object_object.h"
#include "objects/std/frame.h"

using namespace mtpython::objects;
using namespace mtpython::vm;

StdObjSpace::StdObjSpace() : ObjSpace()
{
	typedef_cache = new StdTypedefCache(this);

	builtin_types["int"] = get_typeobject(M_StdIntObject::_int_typedef());
	builtin_types["object"] = get_typeobject(M_StdObjectObject::_object_typedef());
	builtin_types["str"] = get_typeobject(M_StdStrObject::_str_typedef());
	builtin_types["type"] = get_typeobject(StdTypeObject::_type_typedef());
}

mtpython::interpreter::PyFrame* StdObjSpace::create_frame(ThreadContext* context, mtpython::interpreter::Code* code, M_BaseObject* globals)
{
	return new StdFrame(context, code, globals);
}

M_BaseObject* StdObjSpace::lookup_type_cls(M_BaseObject* obj, std::string& attr, M_BaseObject*& where)
{
	StdTypeObject* type = dynamic_cast<StdTypeObject*>(obj);
	if (!type) return nullptr;

	return type->lookup_cls(attr, where);
}

M_BaseObject* StdObjSpace::wrap_int(int x)
{
	return new M_StdIntObject(x);
}

M_BaseObject* StdObjSpace::wrap_int(std::string& x)
{
	return new M_StdIntObject(x);
}

M_BaseObject* StdObjSpace::wrap_str(std::string& x)
{
	return new M_StdStrObject(x);
}
