#include <string>
#include "interpreter/code.h"
#include "objects/std/obj_space_std.h"
#include "objects/std/type_object.h"
#include "objects/std/int_object.h"
#include "objects/std/iter_object.h"
#include "objects/std/list_object.h"
#include "objects/std/bool_object.h"
#include "objects/std/none_object.h"
#include "objects/std/unicode_object.h"
#include "objects/std/dict_object.h"
#include "objects/std/object_object.h"
#include "objects/std/set_object.h"
#include "objects/std/tuple_object.h"
#include "objects/std/bytearray_object.h"
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
	builtin_types["bytearray"] = get_typeobject(M_StdByteArrayObject::_bytearray_typedef());
	builtin_types["dict"] = get_typeobject(M_StdDictObject::_dict_typedef());
	builtin_types["int"] = get_typeobject(M_StdIntObject::_int_typedef());
	builtin_types["object"] = get_typeobject(M_StdObjectObject::_object_typedef());
	builtin_types["set"] = get_typeobject(M_StdSetObject::_set_typedef());
	builtin_types["str"] = get_typeobject(M_StdUnicodeObject::_str_typedef());
	builtin_types["tuple"] = get_typeobject(M_StdTupleObject::_tuple_typedef());
	builtin_types["list"] = get_typeobject(M_StdListObject::_list_typedef());
	builtin_types["type"] = get_typeobject(M_StdTypeObject::_type_typedef());

	make_builtins();
	setup_builtin_modules();
}

mtpython::interpreter::PyFrame* StdObjSpace::create_frame(ThreadContext* context, mtpython::interpreter::Code* code, M_BaseObject* globals)
{
	return new StdFrame(context, code, globals);
}

M_BaseObject* StdObjSpace::lookup(M_BaseObject* obj, const std::string& name)
{
	M_StdTypeObject* obj_type = dynamic_cast<M_StdTypeObject*>(type(obj));
	if (!obj_type) return nullptr;
	
	return obj_type->lookup(name);
}

M_BaseObject* StdObjSpace::lookup_type_cls(M_BaseObject* obj, const std::string& attr, M_BaseObject*& where)
{
	M_StdTypeObject* type = dynamic_cast<M_StdTypeObject*>(obj);
	if (!type) return nullptr;

	type->lock();
	M_BaseObject* value = type->lookup_cls(attr, where);
	type->unlock();

	return value;
}

std::string StdObjSpace::get_type_name(M_BaseObject* obj)
{
	M_StdTypeObject* type_obj = static_cast<M_StdTypeObject*>(type(obj));
	return type_obj->get_name();
}

M_BaseObject* StdObjSpace::wrap_int(ThreadContext* context, int x)
{
	return context->new_object(new M_StdIntObject(x));
}

M_BaseObject* StdObjSpace::wrap_int(vm::ThreadContext* context, const std::string& x)
{
	return context->new_object(new M_StdIntObject(x));
}

M_BaseObject* StdObjSpace::wrap_str(vm::ThreadContext* context, const std::string& x)
{
	return context->new_object(new M_StdUnicodeObject(x));
}

M_BaseObject* StdObjSpace::new_tuple(vm::ThreadContext* context, std::vector<M_BaseObject*>& items)
{
	return context->new_object(new M_StdTupleObject(items));
}

M_BaseObject* StdObjSpace::new_list(vm::ThreadContext* context, std::vector<M_BaseObject*>& items)
{
	return context->new_object(new M_StdListObject(items));
}

M_BaseObject* StdObjSpace::new_dict(vm::ThreadContext* context)
{
	return context->new_object(new M_StdDictObject(this));
}

M_BaseObject* StdObjSpace::new_set(vm::ThreadContext* context)
{
	return context->new_object(new M_StdSetObject(this));
}

M_BaseObject* StdObjSpace::new_seqiter(vm::ThreadContext* context, M_BaseObject* obj)
{
	return context->new_object(new M_StdSeqIterObject(obj));
}

void StdObjSpace::unwrap_tuple(M_BaseObject* obj, std::vector<M_BaseObject*>& list)
{
	M_StdTupleObject* tuple = M_STDTUPLEOBJECT(obj);
	if (!tuple) return;
	std::vector<M_BaseObject*>& items = tuple->get_items();

	list.clear();
	list.insert(list.end(), items.begin(), items.end());
}
