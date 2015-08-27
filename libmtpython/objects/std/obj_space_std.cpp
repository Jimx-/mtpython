#include <string>
#include "interpreter/code.h"
#include "objects/base_object.h"
#include "objects/std/obj_space_std.h"
#include "objects/std/type_object.h"
#include "objects/std/int_object.h"
#include "objects/std/bool_object.h"
#include "objects/std/none_object.h"
#include "objects/std/str_object.h"
#include "objects/std/dict_object.h"
#include "objects/std/object_object.h"
#include "objects/std/tuple_object.h"
#include "objects/std/frame.h"

using namespace mtpython::objects;
using namespace mtpython::vm;

StdObjSpace::StdObjSpace() : ObjSpace()
{
	typedef_cache = new StdTypedefCache(this);

	wrapped_None = new M_StdNoneObject();
	wrapped_True = new M_StdBoolObject(true);
	wrapped_False = new M_StdBoolObject(false);
	
	builtin_types["bool"] = get_typeobject(M_StdBoolObject::_bool_typedef());
	builtin_types["dict"] = get_typeobject(M_StdDictObject::_dict_typedef());
	builtin_types["int"] = get_typeobject(M_StdIntObject::_int_typedef());
	builtin_types["object"] = get_typeobject(M_StdObjectObject::_object_typedef());
	builtin_types["str"] = get_typeobject(M_StdStrObject::_str_typedef());
	builtin_types["tuple"] = get_typeobject(M_StdTupleObject::_tuple_typedef());
	builtin_types["type"] = get_typeobject(StdTypeObject::_type_typedef());

	make_builtins();
}

mtpython::interpreter::PyFrame* StdObjSpace::create_frame(ThreadContext* context, mtpython::interpreter::Code* code, M_BaseObject* globals)
{
	return new StdFrame(context, code, globals);
}

M_BaseObject* StdObjSpace::lookup(M_BaseObject* obj, const std::string& name)
{
	StdTypeObject* obj_type = dynamic_cast<StdTypeObject*>(type(obj));
	if (!obj_type) return nullptr;
	
	return obj_type->lookup(name);
}

M_BaseObject* StdObjSpace::lookup_type_cls(M_BaseObject* obj, const std::string& attr, M_BaseObject*& where)
{
	StdTypeObject* type = dynamic_cast<StdTypeObject*>(obj);
	if (!type) return nullptr;

	type->lock();
	M_BaseObject* value = type->lookup_cls(attr, where);
	type->unlock();

	return value;
}

M_BaseObject* StdObjSpace::wrap_int(int x)
{
	return new M_StdIntObject(x);
}

M_BaseObject* StdObjSpace::wrap_int(const std::string& x)
{
	return new M_StdIntObject(x);
}

M_BaseObject* StdObjSpace::wrap_str(const std::string& x)
{
	return new M_StdStrObject(x);
}

M_BaseObject* StdObjSpace::new_tuple(std::vector<M_BaseObject*>& items)
{
	return new M_StdTupleObject(items);
}

M_BaseObject* StdObjSpace::new_dict()
{
	return new M_StdDictObject(this);
}

void StdObjSpace::unwrap_tuple(M_BaseObject* obj, std::vector<M_BaseObject*>& list)
{
	M_StdTupleObject* tuple = M_STDTUPLEOBJECT(obj);
	if (!tuple) return;
	std::vector<M_BaseObject*>& items = tuple->get_items();

	list.clear();
	list.insert(list.end(), items.begin(), items.end());
}
