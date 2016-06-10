#include "interpreter/code.h"
#include "interpreter/error.h"
#include "interpreter/gateway.h"
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

#include <string>

using namespace mtpython::objects;
using namespace mtpython::vm;
using namespace mtpython::interpreter;

class M_NotImplemented : public M_BaseObject {
public:
	static M_BaseObject* __repr__(ThreadContext* context, mtpython::objects::M_BaseObject* self)
	{
		return context->get_space()->new_interned_str("NotImplemented");
	}

	virtual Typedef* get_typedef();
};

static Typedef NotImplemented_typedef = { "NotImplemented", {
	{ "__repr__", new InterpFunctionWrapper("__repr__", M_StdTypeObject::__repr__) },
}};

Typedef* M_NotImplemented::get_typedef()
{
	return &NotImplemented_typedef;
}

StdObjSpace::StdObjSpace() : ObjSpace()
{
	typedef_cache = new StdTypedefCache(this);

	wrapped_None = new M_StdNoneObject();
	wrapped_True = new M_StdBoolObject(true);
	wrapped_False = new M_StdBoolObject(false);
	wrapped_NotImplemented = new M_NotImplemented();

	/* type of all types */
	M_BaseObject* type_obj = get_typeobject(M_StdTypeObject::_type_typedef());
	type_obj->set_class(this, type_obj);
	builtin_types["type"] = type_obj;

	builtin_types["bool"] = get_typeobject(M_StdBoolObject::_bool_typedef());
	builtin_types["bytearray"] = get_typeobject(M_StdByteArrayObject::_bytearray_typedef());
	builtin_types["dict"] = get_typeobject(M_StdDictObject::_dict_typedef());
	builtin_types["int"] = get_typeobject(M_StdIntObject::_int_typedef());
	builtin_types["object"] = get_typeobject(M_StdObjectObject::_object_typedef());
	builtin_types["set"] = get_typeobject(M_StdSetObject::_set_typedef());
	builtin_types["frozenset"] = get_typeobject(M_StdSetObject::_set_typedef());
	builtin_types["str"] = get_typeobject(M_StdUnicodeObject::_str_typedef());
	builtin_types["tuple"] = get_typeobject(M_StdTupleObject::_tuple_typedef());
	builtin_types["list"] = get_typeobject(M_StdListObject::_list_typedef());

	make_builtins();
	setup_builtin_modules();
}

mtpython::interpreter::PyFrame* StdObjSpace::create_frame(ThreadContext* context, mtpython::interpreter::Code* code, M_BaseObject* globals, M_BaseObject* outer)
{
	return new StdFrame(context, code, globals, outer);
}

M_BaseObject* StdObjSpace::lookup(M_BaseObject* obj, const std::string& name)
{
	if (name == "__subclasshook__") {
		int i = 1;
	}
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

M_BaseObject* StdObjSpace::lookup_type_starting_at(M_BaseObject* type, M_BaseObject* start, const std::string& name)
{
	M_StdTypeObject* as_type = static_cast<M_StdTypeObject*>(type);

	return as_type->lookup_starting_at(start, name);
}

bool StdObjSpace::i_issubtype(M_BaseObject* sub, M_BaseObject* type)
{
	M_StdTypeObject* sub_as_type = dynamic_cast<M_StdTypeObject*>(sub);
	M_StdTypeObject* type_as_type = dynamic_cast<M_StdTypeObject*>(type);

	if (sub_as_type && type_as_type) {
		return sub_as_type->issubtype(type);
	}

	throw InterpError(TypeError_type(), wrap_str(current_thread(), "need type objects"));
}

bool StdObjSpace::i_isinstance(M_BaseObject* inst, M_BaseObject* type)
{
	if (dynamic_cast<M_StdTypeObject*>(type) == nullptr) {
		throw InterpError(TypeError_type(), wrap_str(this->current_thread(), "need type object"));
	}
	return static_cast<M_StdTypeObject*>(this->type(inst))->issubtype(type);
}

M_BaseObject* StdObjSpace::get_type_by_name(const std::string& name)
{
	auto found = builtin_types.find(name);
	if (found == builtin_types.end())
		return nullptr;

	return found->second;
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

M_BaseObject* StdObjSpace::wrap_int(ThreadContext* context, const std::string& x)
{
	return context->new_object(new M_StdIntObject(x));
}

M_BaseObject* StdObjSpace::wrap_str(ThreadContext* context, const std::string& x)
{
	return context->new_object(new M_StdUnicodeObject(x));
}

M_BaseObject* StdObjSpace::new_tuple(ThreadContext* context, const std::vector<M_BaseObject*>& items)
{
	return context->new_object(new M_StdTupleObject(items));
}

M_BaseObject* StdObjSpace::new_list(ThreadContext* context, const std::vector<M_BaseObject*>& items)
{
	return context->new_object(new M_StdListObject(items));
}

M_BaseObject* StdObjSpace::new_dict(ThreadContext* context)
{
	return context->new_object(new M_StdDictObject(this));
}

M_BaseObject* StdObjSpace::new_set(ThreadContext* context)
{
	return context->new_object(new M_StdSetObject(this));
}

M_BaseObject* StdObjSpace::new_seqiter(ThreadContext* context, M_BaseObject* obj)
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

int StdObjSpace::i_get_index(M_BaseObject* obj, M_BaseObject* exc, M_BaseObject* descr)
{
	int result;
	try {
		result = obj->to_int(this, false);
	} catch (const NotImplementedException&) {
		return ObjSpace::i_get_index(obj, exc, descr);
	}

	return result;
}

